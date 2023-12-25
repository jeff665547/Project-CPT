# This program is used to modify the C++ hpp file's "include guard" from "pragma once" to "define macro"

import os
from StringIO import StringIO
import sys
# rootdir = '../change_include_guard'
def mod_rule( s ):
    return ( s.replace('_', '__').replace('.', '_').upper() );

def handle_file ( file ):
    # ffname = os.path.abspath(os.path.join(subdir, file))
    ffname = os.path.abspath( file )
    if '.hpp' not in ffname: return ;
    guard_name = '';
    record = False;
    sfname = ffname.split('/');
    for i, s in enumerate(sfname):
        if record == True:
            guard_name += ('_' + mod_rule( s));
        if s == 'src':
            record = True;
            guard_name += mod_rule(sfname[i-1]);
    print ffname;
    # print guard_name;
    buf = StringIO();
    with open(ffname, 'r+') as srcf:
        mod_guard = False;
        for line in srcf.readlines():
            if '#pragma once' in line:
                buf.write('#ifndef ' + guard_name + '\n');
                buf.write('#define ' + guard_name + '\n');
                mod_guard = True;
                continue;
            else:
                buf.write(line);
        if mod_guard == True:
            buf.write('#endif');
        buf.seek(0);
        srcf.seek(0);
        srcf.write(buf.read());

def run( rootdir ):
    if os.path.isfile(rootdir):
        handle_file(rootdir)
    elif os.path.isdir(rootdir):
        for subdir, dirs, files in os.walk(rootdir):
            for file in files:
                handle_file(os.path.join(subdir, file))
    else:
        raise NotImplementedError('path : %s, unknown path type' % rootdir);

            
if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit('Usage: %s [root directory]' % sys.argv[0]);
    else:
        sys.exit(run(sys.argv[1]));
