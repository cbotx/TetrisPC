from sympy import *
from sympy.parsing.sympy_parser import parse_expr

def read_chunks(lines):
    chunk = []
    for line in lines:
        if not line:
            if len(chunk):
                yield chunk
                chunk.clear()
        else:
            chunk.append(line)

def get_expression(x, y):
    shift_distance = y * 16 - x

    s = ""
    if shift_distance > 0:
        s = "<<"
    elif shift_distance < 0:
        s = ">>"
    else:
        return Symbol("h")
    return Symbol(f"h{s}{abs(shift_distance)}")

def chunk_get_formula(chunk):
    formula = 0
    piece_formula = 0
    wall_dict = {}
    wall_formula = 0
    bottom_formula = 1
    mx_x = 0
    mx_y = 0
    for i in range(len(chunk)):
        line = chunk[i]
        for j in range(len(line)):
            if line[j] == 'x':
                mx_x = max(mx_x, j)
                mx_y = max(mx_y, i)
    
    
    for i in range(len(chunk)):
        line = chunk[i]
        for j in range(len(line)):
            if line[j] == 'x':
                piece_formula = Or(piece_formula, get_expression(mx_x - j, mx_y - i))

    for i in range(len(chunk)):
        line = chunk[i]
        for j in range(len(line)):
            if line[j] != ' ':
                if line[j] == 'x':
                    flag = True
                    if len(chunk) > i + 1 and len(chunk[i + 1]) > j:
                        if chunk[i + 1][j] != ' ':
                            flag = False
                    if flag:
                        bottom_formula = And(bottom_formula, Not(get_expression(mx_x - j, mx_y - (i + 1))))
                elif line[j] == '.':
                    formula = Or(formula, get_expression(mx_x - j, mx_y - i))
                else:
                    ch = line[j]
                    if ch not in wall_dict.keys():
                        wall_dict[ch] = 1

                    wall_dict[ch] = And(wall_dict[ch], Not(get_expression(mx_x - j, mx_y - i)))
    for _, val in wall_dict.items():
        wall_formula = Or(wall_formula, val)
    
    return Or(formula, wall_formula), Or(piece_formula, bottom_formula)

def read_rules(filename):
    ori = -1
    formula = 1
    lines = open(filename, "r").read().splitlines()
    shared_f = 0
    for chunk in read_chunks(lines):
        if chunk[0].isnumeric():
            if formula != 1:
                yield Or(shared_f, formula)
                formula = 1
            ori = int(chunk[0])
            print(ori)
        elif chunk[0][0] == '=':
            if formula != 1:
                yield Or(shared_f, formula)
                formula = 1
            print(chunk[0])
        else:
            f, shared_f = chunk_get_formula(chunk)
            formula = And(formula, f)
    if formula != 1:
        yield Or(shared_f, formula)


if __name__ == "__main__":
    for rule in read_rules("drop_rules.txt"):
        print(rule)
