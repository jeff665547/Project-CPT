#!/usr/bin/python3
import json
import sys
import os
from functools import partial
def include_path_fix(pf, icf):
    return os.path.join(
            os.path.dirname(
                os.path.abspath(pf)
            ), 
            icf
        )

def access_by_query( r, q ):
    res = r;
    # print(q);
    for tok in q[1:]:
        if type(res) is list:
            res = res[int(tok)]
        else:
            res = res[tok]
    return res

def handler( r, value, vh ):
    if   type(value) is list:
        return handle_list (r, value, vh)
    elif type(value) is dict:
        return handle_dict (r, value, vh)
    else:
        return handle_value(r, value, vh)

def do_variables( r, s ):
    if type(s) is str:
        if len(s) > 0 and s[0] == '$':
            query = s.split('.')
            return access_by_query( r, query )
        else: return s
    else: return s;

def do_file( name, r, s ):
    if type(s) is str:
        if len(s) > 0 and s[0] == '@':
            return file2dict(include_path_fix(name, s[1:]))
        else: return s
    else: return s;

def handle_value  (r, s, vh):
    return vh( r, s )

def handle_list (r, l, vh):
    for i, value in enumerate(l):
        l[i] = handler(r, value, vh)
    return l;

def handle_dict (r, d, vh):
    for key, value in d.items():
        d[key] = handler(r, value, vh)
    return d;
def preprocess(name):
    res = ""
    # print ( name )
    f = open(str(name), 'r')
    lines = f.readlines();
    for i, s in enumerate(lines):
        lines[i] = s.split('//')[0];
        res += lines[i]
    d_in = json.loads(res)
    return d_in;

# def dict2dict(d_in):
#     return handler( d_in, d_in )

def file2dict(name):
    d_in = preprocess(name)
    d_in = handler( d_in, d_in, partial(do_file, name))
    d_in = handler( d_in, d_in, do_variables)
    return d_in

def file2string(name):
    return json.dumps(file2dict(name))

def main(args):
    if len(args) == 2:
        print ( file2string( args[1] ) )
    else:
        print ( 'json_templar.py [json name]' )

if __name__ == "__main__":
    main(sys.argv)
