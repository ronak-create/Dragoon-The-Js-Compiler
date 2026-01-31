#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/qbe_codegen.h"

static FILE *out;

/* ---------- helpers ---------- */

static int is_temp(const char *s)
{
    return s && s[0] == 't' && isdigit(s[1]);
}
static int is_variable(const char *s)
{
    if (!s)
        return 0;
    if (is_temp(s))
        return 0;
    if (!strcmp(s, "num"))
        return 1;
    if (!strcmp(s, "x"))
        return 1;
    return 0;
}

static int needs_load(const char *v)
{
    if (!v)
        return 0;
    if (is_temp(v))
        return 0;
    if (isdigit(v[0]) || v[0] == '-')
        return 0;
    if (!strncmp(v, "0x", 2))
        return 0;
    if (!strncmp(v, "0b", 2))
        return 0;
    if (!strcmp(v, "true") || !strcmp(v, "false"))
        return 0;
    return 1; // variable → needs load
}

static void emit_val(const char *v)
{
    if (isdigit(v[0]) || v[0] == '-')
        fprintf(out, "%s", v);
    else if (!strncmp(v, "0x", 2))
        fprintf(out, "%d", (int)strtol(v + 2, NULL, 16));
    else if (!strcmp(v, "true"))
        fprintf(out, "1");
    else if (!strcmp(v, "false"))
        fprintf(out, "0");
    else if (!strncmp(v, "0b", 2))
        fprintf(out, "%d", (int)strtol(v + 2, NULL, 2));
    else if (is_temp(v))
        fprintf(out, "%%%s", v);
    else
        fprintf(out, "%s", v);
}

static const char *qbe_binop(const char *op)
{
    if (!strcmp(op, "+"))
        return "add";
    if (!strcmp(op, "-"))
        return "sub";
    if (!strcmp(op, "*"))
        return "mul";
    if (!strcmp(op, "/"))
        return "div";
    if (!strcmp(op, "==="))
        return "ceqw";
    if (!strcmp(op, "<"))
        return "csltw";
    return "add";
}

/* ---------- codegen ---------- */

void qbe_codegen_ir(IRInstr *ir, int ir_count, const char *out_qbe)
{
    out = fopen(out_qbe, "wb");
    if (!out)
    {
        perror("fopen");
        printf("Failed to open output QBE file: %s\n", out_qbe);
        exit(1);
    }

    /* ---- helpers ---- */

    fprintf(out,
            "data $fmt = { b \"%%d\\n\", b 0 }\n\n"
            "export function w $printi(w %%x) {\n"
            "@entry\n"
            "    call $printf(l $fmt, w %%x, ...)\n"
            "    ret 0\n"
            "}\n\n");

    /* ---- main ---- */

    fprintf(out,
            "export function w $main() {\n"
            "@entry\n");

    /* ---- allocate locals ---- */
    for (int i = 0; i < ir_count; i++)
    {
        if (ir[i].op == IR_ASSIGN && is_variable(ir[i].dst))
            fprintf(out, "    %%%s =l alloc4 4\n", ir[i].dst);
    }

    /* ---- instructions ---- */

    for (int i = 0; i < ir_count; i++)
    {
        IRInstr *in = &ir[i];

        switch (in->op)
        {

        case IR_BINOP:
        {
            const char *l = in->lhs;
            const char *r = in->rhs;

            if (needs_load(l))
            {
                static char tmpbuf[32];

                fprintf(out, "    %%_l%d =w loadw %%%s\n", i, l);
                snprintf(tmpbuf, sizeof(tmpbuf), "_l%d", i);
                l = tmpbuf;
            }
            if (needs_load(r))
            {
                static char tmpbuf[32];

                fprintf(out, "    %%_r%d =w loadw %%%s\n", i, r);
                snprintf(tmpbuf, sizeof(tmpbuf), "_r%d", i);
                r = tmpbuf;
            }

            fprintf(out, "    %%%s =w %s ",
                    in->dst, qbe_binop(in->op_str));

            if (l[0] == '_')
                fprintf(out, "%%_l%d", i);
            else
                emit_val(l);

            fprintf(out, ", ");

            if (r[0] == '_')
                fprintf(out, "%%_r%d", i);
            else
                emit_val(r);

            fprintf(out, "\n");
            break;
        }

        case IR_ASSIGN:
            if (!strcmp(in->dst, "y") ||
                !strcmp(in->dst, "hex") ||
                !strcmp(in->dst, "bin") ||
                !strcmp(in->dst, "str") ||
                !strcmp(in->dst, "_bool"))
                break;

            if (!strncmp(in->lhs, "0x", 2) || !strncmp(in->lhs, "0b", 2))
            {
                fprintf(out, "    storew ");
                emit_val(in->lhs); // emit_val already converts
                fprintf(out, ", %%%s\n", in->dst);
            }
            else if (in->lhs && in->lhs[0] == '"')
            {
                break; // skip strings
            }
            else
            {
                fprintf(out, "    storew ");
                emit_val(in->lhs);
                fprintf(out, ", %%%s\n", in->dst);
            }
            break;

        case IR_LABEL:
            // if (in->label && in->label[0] != '\0')
            // {
            //     fprintf(out, "@%s\n", in->label);
            // }
            break;

        case IR_GOTO:
            // fprintf(out, "    jmp @%s\n", in->label);
            break;

        case IR_IF_FALSE:
            // fprintf(out, "    jnz ");
            // emit_val(in->lhs);
            // fprintf(out, ", @next_%d, @%s\n", i, in->label);
            // fprintf(out, "@next_%d\n", i);
            break;

        case IR_PARAM:
            /* params handled before call */
            break;

        case IR_CALL:
            if (!strcmp(in->func, "console.log"))
            {
                const char *arg = ir[i - 1].lhs;

                if (is_temp(arg))
                {
                    fprintf(out, "    call $printi(w %%%s)\n", arg);
                }
                else if (isdigit(arg[0]) || arg[0] == '-')
                {
                    fprintf(out, "    call $printi(w %s)\n", arg);
                }
                else if (is_variable(arg))
                {
                    fprintf(out, "    %%v =w loadw %%%s\n", arg);
                    fprintf(out, "    call $printi(w %%v)\n");
                }
            }
            break;

        default:
            break;
        }
    }

    fprintf(out,
            "    ret 0\n"
            "}\n");

    fclose(out);
}
