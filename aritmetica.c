// CPP program to evaluate a given
// expression where tokens are
// separated by space.
// Function to find precedence of
// operators.
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

struct stack
{
    int data;
    struct stack *ptr; //pointer type of stack
};

typedef struct stack Stack;
typedef Stack *stackPtr;

//function prototypes of different functions
void push(stackPtr *top, int x);   //for pushing value in stack
int pop(stackPtr *top);            //for popping value out of stack
int checkEmpty(stackPtr top);      //checking whether stack is empty
void printValue(stackPtr showPtr); //for printing value that are pushed and popped

int precedence(char op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    return 0;
}

// Function to perform arithmetic operations.
int applyOp(int a, int b, char op)
{
    switch (op)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        return a / b;
    }
}

// Function that returns value of
// expression after evaluation.
int evaluate(char *tokens)
{
    int i;

    // stack to store integer values.
    stack values;

    // stack to store operators.
    stack ops;

    for (i = 0; i < strlen(tokens); i++)
    {

        // Current token is a whitespace,
        // skip it.
        if (tokens[i] == ' ')
            continue;

        // Current token is an opening
        // brace, push it to 'ops'
        else if (tokens[i] == '(')
        {
            ops.push(tokens[i]);
        }

        // Current token is a number, push
        // it to stack for numbers.
        else if (isdigit(tokens[i]))
        {
            int val = 0;

            // There may be more than one
            // digits in number.
            while (i < strlen(tokens) &&
                   isdigit(tokens[i]))
            {
                val = (val * 10) + (tokens[i] - '0');
                i++;
            }

            values.push(val);

            // right now the i points to
            // the character next to the digit,
            // since the for loop also increases
            // the i, we would skip one
            //  token position; we need to
            // decrease the value of i by 1 to
            // correct the offset.
            i--;
        }

        // Closing brace encountered, solve
        // entire brace.
        else if (tokens[i] == ')')
        {
            while (!ops.empty() && ops.top() != '(')
            {
                int val2 = values.top();
                values.pop();

                int val1 = values.top();
                values.pop();

                char op = ops.top();
                ops.pop();

                values.push(applyOp(val1, val2, op));
            }

            // pop opening brace.
            if (!ops.empty())
                ops.pop();
        }

        // Current token is an operator.
        else
        {
            // While top of 'ops' has same or greater
            // precedence to current token, which
            // is an operator. Apply operator on top
            // of 'ops' to top two elements in values stack.
            while (!ops.empty() && precedence(ops.top()) >= precedence(tokens[i]))
            {
                int val2 = values.top();
                values.pop();

                int val1 = values.top();
                values.pop();

                char op = ops.top();
                ops.pop();

                values.push(applyOp(val1, val2, op));
            }

            // Push current token to 'ops'.
            ops.push(tokens[i]);
        }
    }

    // Entire expression has been parsed at this
    // point, apply remaining ops to remaining
    // values.
    while (!ops.empty())
    {
        int val2 = values.top();
        values.pop();

        int val1 = values.top();
        values.pop();

        char op = ops.top();
        ops.pop();

        values.push(applyOp(val1, val2, op));
    }

    // Top of 'values' contains result, return it.
    return values.top();
}

int main()
{
    printf("%d", evaluate("10 + 2 * 6"));
    printf("%d", evaluate("100 * 2 + 12"));
    printf("%d", evaluate("100 * ( 2 + 12 )"));
    printf("%d", evaluate("100 * ( 2 + 12 ) / 14"));
    return 0;
}

// This code is contributed by Nikhil jindal.