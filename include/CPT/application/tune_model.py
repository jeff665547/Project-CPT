#!/usr/bin/python3

# workdir
# config template
# modify parameter 
# training ( record time )
# calling
# analysis result ( cen_affy_compare )

import json
import sys
import os
import io
import subprocess
import re
import tempfile
def read_config_tpl( tpl_abs_path ): 
    # read config return data structure
    return json.load(
        open(tpl_abs_path)
    );

def create_train_config( para, config_tpl ): 
    # fill parameter to config template 
    config_tpl["input"]["clustering_models"][0] = para["clustering_model"]
    for i in range(len(config_tpl["pipeline"])):
        if ( config_tpl["pipeline"][i]["name"] == para["name"] ):
            for k, v in para["parameter"].items():
                config_tpl["pipeline"][i]["parameter"][k] = v
    return config_tpl

def create_call_config( para, config_tpl ):
    config_tpl["input"]["clustering_models"][0] = para["clustering_model"]
    config_tpl["output_dir"] = para["call_res_dir"]
    return config_tpl

def lastline( ios ):
    for line in ios:
        pass
    print ( "last line : " + line )
    return line;
def lastline2( tf ):
    for line in tf:
        pass
    res = line.decode()
    print ( "last line : " + res )
    return res;
def training( param ):
    workdir          = param["training_workdir"]
    config_tpl_path  = param["training_config_tpl_path"]
    exe              = param["training_exe"]

    os.chdir( workdir )

    log         = tempfile.NamedTemporaryFile();
    ctpl        = read_config_tpl( config_tpl_path )
    config_str  = json.dumps(
        create_train_config( param, ctpl )
    )
    print( "train log file : " + log.name )
    p = subprocess.Popen(
        exe, 
        shell       = True, 
        stdin       = subprocess.PIPE, 
        stdout      = log, 
        close_fds   = True,
    )
    out_s, err_s = p.communicate( input=config_str.encode() )
    print( "train finish, start collect result" )
    log.seek(0)
    param["training_time"] = json.loads( lastline2( log ) )["Component Birdseed Probeset Training"]["time"]
    return param

def calling( param ):
    workdir         = "/home/john/workdir/CPT/birdseed/absp_call"
    config_tpl_path = "config/50000test.json"
    exe             = "bin/pipeline_builder_dbg"
    print( "call start, change workdir");
    os.chdir( workdir )
    log         = tempfile.NamedTemporaryFile(delete=False);
    print( "call log file : " + log.name )
    ctpl        = read_config_tpl(os.path.join( workdir, config_tpl_path ))
    config_str  = json.dumps(
        create_call_config( param, ctpl )
    )
    if not os.path.exists( param["call_res_dir"] ):
        os.makedirs( param["call_res_dir"] )
    print( "call pipeline starting" )
    p = subprocess.Popen(
        os.path.join( workdir, exe ), 
        shell       = True, 
        stdin       = subprocess.PIPE, 
        stdout      = log,
        close_fds   = True,
    )
    out_s, err_s = p.communicate( input=config_str.encode() )
    print( "call pipeline finish" )
    return param

def grep ( genotype, pattern ):
    print ( genotype )
    genotype_qcp = os.path.join( 
        os.path.dirname( os.path.abspath( genotype ) ),
        'genotype_qc.tsv'
    )
    genotype_qc_s   = open(genotype_qcp, 'w')
    genotype_s      = open(genotype, 'r')
    for line in genotype_s:
        if re.match( pattern, line ):
            genotype_qc_s.write( line );
    return genotype_qcp
            
def analysis( param ):
    workdir         = "/home/john/workdir/CPT/birdseed/absp_call"
    exe             = "bin/cen_affy_compare"
    gold_standard   = "/home/alex/data/array/GSE78098/results_161020/AxiomGT1.calls.txt"
    genotypep       = os.path.join( param["call_res_dir"], "genotype.tsv" )
    
    cmd = (
        os.path.join( workdir, exe ) + " " 
        + genotypep + " "
        + gold_standard + " "
        + os.path.join( param["call_res_dir"], "all" )
    )
    print( "apt \"all\" analysis start" );
    print( cmd )
    p = subprocess.Popen(
        cmd, 
        shell = True, 
        close_fds = True,
        stdout = open('/dev/null'),
        stderr = open('/dev/null')
    )
    p.wait()
    print( "apt \"all\" analysis finish, start collect result" );
    param["all_acc"] = re.findall(
        "\d+\.\d+",
        lastline(
            open(os.path.join(
                param["call_res_dir"], 
                "all_cac_conf_matrix"
            ))
        )
    )[0]
    print( "apt \"all\" collect done" );
    genotype_qcp =  grep( genotypep, ".*AFFX.*" )
    cmd = ( 
        os.path.join( workdir, exe ) + " " 
        + genotype_qcp + " "
        + gold_standard + " "
        + os.path.join( param["call_res_dir"], "qc" )
    )
    print( "apt \"qc\" analysis start" );
    print( cmd )
    p = subprocess.Popen(
        cmd, 
        shell = True, 
        close_fds = True,
        stdout = open('/dev/null'),
        stderr = open('/dev/null')
    )
    p.wait()
    print( "apt \"qc\" analysis finish, start collect result" );
    param["qc_acc"] = re.findall(
        "\d+\.\d+",
        lastline(
            open(os.path.join(
                param["call_res_dir"], 
                "qc_cac_conf_matrix"
            ))
        )
    )[0]
    print( "apt \"qc\" collect done" );
    return param
def view_header(path):
    out = open( path, 'w+');
    if not os.path.exists(path) :
        out.close()
        out = sys.stdout;
    out.write("param_path\tqc_acc\tall_acc\ttraining_time\n");
    return out;
    
def view(param, out):
    print("view: start write result")
    param_path      = param["param_path"];
    qc_acc          = param["qc_acc"];
    all_acc         = param["all_acc"];
    training_time   = param["training_time"];
    out.write("{0}\t{1}\t{2}\t{3}\n".format(param_path, qc_acc, all_acc, training_time));
    print("view: one record is added")
def view_mock(param):
    param["qc_acc"] = 0;
    param["all_acc"] = 0;
    param["training_time"] = 0;

def main( cmd_args ):
    if len(cmd_args) == 3:
        param_list = json.load(open(cmd_args[1]));
        result_path = cmd_args[2]

        res = []
        view_s = view_header(result_path);
        for param in param_list:
            param["param_path"] = cmd_args[1];
            training    ( param )
            print( "result collect done" )
            calling     ( param )
            print( "result collect done" )
            analysis    ( param )
            print( "result collect done" )
            view        ( param, view_s);
            print( "result collect done" )
            res.append  ( param )
        return 0;
    else:
        print ( "tune_model.py [parameter.json] [result path]" );
        return 1;
        
    
if __name__ == "__main__":
    sys.exit(main(sys.argv));
    # print ( json.dumps(main(sys.argv)) )
