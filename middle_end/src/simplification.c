#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <simplification.h>

#define MAX_OPTIMIZATIONS 100

#define _IS(where, what) (root && root->where &&         \
                          root->where->type  ==  NUM  && \
                          (int) root->where->value == (what))

#define $ verificator(root, __FILE__, __LINE__);

double eval (struct Node_t* node)
{
#ifdef DEBUG
    fprintf (stderr, "Starting evaluation...\n");
#endif

    if (node == NULL)
    {
#ifdef DEBUG
        fprintf (stderr, "ERROR: Node is NULL\n");
#endif
        return NAN;
    }

    if (node->type == NUM)
    {
        double value = node->value;
#ifdef DEBUG
        fprintf (stderr, "node->type = NUM >>> node->value = %lg\n\n", node->value);
#endif

        return value;
    }

    if (node->type == ID)
    {
#ifdef DEBUG
        fprintf (stderr, "node->type = ID >>> node->value = %c\n\n", (int) node->value);
#endif
        return NAN;
    }

    if (node->type == OP)
    {
#ifdef DEBUG
        fprintf (stderr, "node->type = OP >>> node->value = %c\n\n", (int) node->value);
#endif

        switch ( (int) node->value )
        {
            case ADD:
            {
                double res = eval (node->left) +
                             eval (node->right);

#ifdef DEBUG
                fprintf (stderr, "case ADD: result = %lg\n\n", res);
#endif
                return res;
            }

            case SUB:
            {
                double res = eval (node->left) -
                             eval (node->right);
#ifdef DEBUG
                fprintf (stderr, "case SUB: result = %lg\n\n", res);
#endif
                return res;
            }

            case DIV:
            {
                double res = eval (node->left) /
                             eval (node->right);
#ifdef DEBUG
                fprintf (stderr, "case DIV: result = %lg\n\n", res);
#endif
                return res;
            }

            case MUL:
            {
                double res = eval (node->left) *
                             eval (node->right);
#ifdef DEBUG
                fprintf (stderr, "case MUL: result = %lg\n\n", res);
#endif
                return res;
            }

            case SIN:
            {
                double res = sin ( eval (node->left) );
#ifdef DEBUG
                fprintf (stderr, "case SIN: result = %lg\n\n", res);
#endif
                return res;
            }

            case COS:
            {
                double res = cos ( eval (node->left) );
#ifdef DEBUG
                fprintf (stderr, "case COS: result = %lg\n\n", res);
#endif
                return res;
            }

            default:
            {
#ifdef DEBUG
                fprintf (stderr, "Unknown operation. returned 1");
#endif
                return 1;
            }
        }
    }
    else
    {
#ifdef DEBUG
        fprintf (stderr, "ERROR with evaluation!!! Thats not a number, id or operation\n");
#endif
        return NAN;
    }
}

int simplification_typical_operations (struct Node_t* root, struct Node_t* parent)
{
    assert (root);

    int count_changes = 0;

    if (root->left)
        count_changes += simplification_typical_operations (root->left, root);

    if (root->right)
        count_changes += simplification_typical_operations (root->right, root);

    if ( (root->type == OP) && ((int) root->value == MUL ))
    {
        if ( _IS (left, 0) || _IS (right, 0) )
        {
            //dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            delete_sub_tree (root->left);
            delete_sub_tree (root->right);

            root->type  = NUM;
            root->value = 0;

            root->left  = NULL;
            root->right = NULL;

            //dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            return count_changes;
        }
    }

    if ( (root->type == OP) && ((int) root->value == MUL) )
    {
        if ( _IS (left, 1) )
        {
            //dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            if (parent->left == root)
                parent->left  = root->right;
            else
                parent->right = root->right;

            delete_node (root->left);
            delete_node (root);

            //dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            return count_changes;
        }

        if ( _IS (right, 1) )
        {
            //dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            if (parent->left == root)
                parent->left  = root->left;
            else
                parent->right = root->left;

            delete_node (root->right);
            delete_node (root);

            //dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            return count_changes;
        }
    }

    if ( (root->type == OP) && ((int) root->value == ADD))
    {
        if ( _IS (left, 0) )
        {
$           if (parent->left == root)
                parent->left  = root->right;
            else
                parent->right = root->right;
$
            //dump_in_log_file (parent, "<h1>BEFORE DELETE ADD left:</h1>", root->left, root);

            delete_node (root->left);
            delete_node (root);

            //dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node [%p] :</h1>", root->left, root);

            return count_changes;
        }

        if ( _IS (right, 0) )
        {
            //dump_in_log_file (parent, "<h1>BEFORE DELETE ADD right");

            if (parent->left == root)
                parent->left  = root->left;
            else
                parent->right = root->left; //func

            delete_node (root->right);
            delete_node (root);

            //dump_in_log_file (parent, "<h1>delete node->left [%p], delete node [%p]:</h1>", root->left, root->right);

            return count_changes;
        }
    }

    if ( (root->type == OP) && ((int) root->value == POW ))
    {
        if ( _IS (right, 1) )
        {
            //dump_in_log_file (parent, "<h1>BEFORE DELETE POW</h1>");

            if (parent->left == root)
                parent->left  = root->left;
            else
                parent->right = root->left;

            delete_node (root->right);
            delete_node (root);

            //dump_in_log_file (parent, "<h1>AFTER DELETE POW</h1>");

            return count_changes;
        }
    }

    return count_changes;
}

#undef $
#undef _IS

void verificator (struct Node_t* node, const char* filename, int line)
{
    if (node->type == 0)
        fprintf (stderr, "%s:%d: vasalam u have a problem: node [%p]: type = %d, value = %c (%lg)\n\n",
                 filename, line, node, node->type, (int) node->value, node->value);

    if (node->left)  verificator (node->left, filename, line);
    if (node->right) verificator (node->right, filename, line);
}

int constant_folding (struct Node_t* root)
{
    assert (root);

    int count_changes = 0;

    if (root->left)
        count_changes += constant_folding (root->left);

    if (root->right)
        count_changes += constant_folding (root->right);

    if (root &&
        root->left  != NULL &&
        root->right != NULL &&
        root->type == OP &&
        root->left->type  == NUM &&
        root->right->type == NUM)
    {
        double answer = eval (root);
#ifdef DEBUG
        fprintf (stderr, "\nlol im in if: node [%p]: type=%d, left=%g, right=%g, answer = %lg\n\n",
                 root, root->type, root->left->value, root->right->value, answer);
#endif

        struct Node_t* left_to_delete  = root->left;
        struct Node_t* right_to_delete = root->right;

        root->type = NUM;
        root->value = answer;

#ifdef DEBUG
        fprintf (stderr, "Deleting left subtree [%p] and right subtree [%p] from node [%p]\n",
                 left_to_delete, right_to_delete, root);
#endif

        delete_sub_tree (left_to_delete);
        delete_sub_tree (right_to_delete);

        root->left  = NULL;
        root->right = NULL;

        count_changes++;
    }
    else
    {
#ifdef DEBUG
        fprintf (stderr, "Skipped if for node [%p]: type=%d, left_type=%d, right_type=%d\n",
                 root, root->type, root->left ? root->left->type : -1, root->right ? root->right->type : -1);
#endif
    }

    return count_changes;
}

int simplification_of_expression (struct Node_t* root, struct Node_t* parent)
{
    for (int i = 0; i < MAX_OPTIMIZATIONS; i++)
    {
        int changes = 0;

        changes += simplification_typical_operations (root, parent);
        changes += constant_folding (root);

#ifdef DEBUG
        fprintf (stderr, "Changes = %d\n\n", changes);
#endif

        if (changes == 0)
            break;
    }

    return 0;
}
