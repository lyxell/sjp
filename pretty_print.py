import sys

data = ""

for line in sys.stdin:
    data += line

indent = 0

def print_indent():
    for i in range(indent):
        print(' ', end="")

for c in data:
    if c == '\n': continue
    if c == ']':
        indent -= 4
        print()
        print_indent()
    print(c, end="")
    if c == '[':
        indent += 4
        print()
        print_indent()
