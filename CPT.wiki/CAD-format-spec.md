### Table of Contents
1. [**CAD Schema**](#cad-schema)
2. [**CAD Spec in Details**](#cad-spec-in-details)
3. [**Examples**](#examples)
4. [**Minimal Example**](#minimal-example)
5. [**Default Usage**](#default-usage)
6. [**Tag-based Genotyping**](#tag-based-genotyping)
7. [**Ligation-based Genotyping**](#ligation-based-genotyping)
8. [**HDF5 header for CAD**](#hdf5-header-for-cad)

### CAD Schema
***

The following defines the CAD file hierarchy in JSON format. Brief description is attached after "#" with corresponding data type and requirement. More details can be referred in footnotes and examples.

        {
            "magic": 113                              # [int32 ] CAD file signature (required)
          , "version": "000"                          # [string] CAD file version (required)
          , "probe_array_type":                       # probe array type
            {
                "name": "example"                     # [string] chip identifier (required)
              , "version": 0                          # [string] chip version (required)
            }
          , "num_probesets": 0                        # [uint32] number of unique probesets (required)
          , "num_features": 0                         # [uint32] number of unique features (required)
          , "num_rows": 0                             # [uint16] number of feature rows (optional, default = 0) [1]
          , "num_cols": 0                             # [uint16] number of feature columns (optional, default = 0) [2]
          , "num_channels": 1                         # [uint16] number of channels used (optional, default = 1) [3]
          , "max_chn_items": 2                        # [uint16] max channel description items (optional, default = 1) [4]
          , "max_seq_length": 40                      # [uint16] max probe sequence length (required)
          , "genome_assembly": "(unused)"             # [string] version of genome assembly (optional, default = "(unused)") [5]
          , "probe_direction": "3-5"                  # [string] order of nucleotide for sequence content (optional, default = "3-5") [6]
          , "probeset_list":                          # a list of probeset descriptions (required)
            [
                # probeset definition
                {
                    "name": ""                        # [string] probeset identifier (required)
                  , "type": ""                        # [string] type of assay (required) [7]
                  , "subtype": ""                     # [string] type of detection method (required) [8]
                  , "chrom": ""                       # [string] name of chromosome (optional, default = "---") [9]
                  , "start": 0                        # [uint32] starting position of the feature in chromosome (optional, default = 0) [10][12]
                  , "end": 0                          # [uint32] ending position of the feature in chromosome (optional, default = 0) [11][12]
                  , "strand": "."                     # [char  ] strand of the target base in reference genome (optional, default = ".")
                  , "desc": ""                        # [string] variation type (required if type = "Genotyping"; otherwise, this field is optional; default = "---") [13]
                  , "num_probes": 0                   # [uint16] number of probes within this probeset
                  , "probe_list":                     # a list of probe descriptions (required)
                    [
                        # probe definition
                        {
                            "probe_name": 0           # [uint32] probe identifier (required) [14]
                          , "shape_name": 0           # [uint16] identifier to physcical probe design pattern (required) [15]
                          , "region_des":             # specifies the location and dimensions of the probe placement (required)
                            [
                                0                     # [uint32] x cooridnate/grid (required)
                              , 0                     # [uint32] y cooridnate/grid (required)
                              , 1                     # [uint16] x dimension (optional, default = 1)
                              , 1                     # [uint16] y dimension (optional, default = 1)
                            ]
                          , "channel_des":            # channel description at substitution/extension position (optional)
                            {
                                "allele": " A/B/C/D"  # [string] allele coding (default = "---")
                              , "channel": "0/2/1/3"  # [string] channel coding (default = "---")
                              , "base": "A/C/G/T"     # [string] base of substitution/extension (default = "---")
                            }
                          , "sequence":               # sequence content of probe (optional)
                            {
                                "start": 0            # [uint32] starting position of the sequence in genome reference (default = 0) [16]
                              , "length": 0           # [uint16] probe lengthi (default = 0)
                              , "strand": "."         # [char  ] strand of the probe sequence corresponding to the genome reference (default = '.')
                              , "content": "---"      # [string] sequence content (default = "---")
                            }
                        }
                    ]
                }
            ]
        }

Footnote:
1. "num_rows" is optional if the chip is not grid-based design.
2. "num_cols" is optional if the chip is not grid-based design.
3. "num_channels" is optional if the chip is single channel design.
4. "max_chn_items" is optional if the chip is single channel design.
5. Set "genome_assembly" to "(unused)" if unused.
6. "probe_direction" refers to the order of nucleotide of probe sequence content. The value should be either '3-5' or '5-3'.
7. Possible choice of "type" should be "Expression", "Copynumber", "Genotyping" and "Sequencing".
8. Possible choice of "subtype" should be either "TagBased", "LigationBased", "PolymeraseExtensionBased" or "Sequencing".
9. Please set "chrom" to the name of reference sequence, or "---" if unused.
10. The starting position is __0-base inclusive__ with corresponding to __positive__ strand of reference genome. Please set the value to 0 if unused.
11. The ending position is __0-base exclusive__ with corresponding to __positive__ strand of reference genome. Please set the value to 0 if unused.
12. In the 0-based format, the starting position cannot be equal to the ending position
13. "desc" refers to nucleotides of the base of target SNP.
14.  "probe_name" refers to the probe identifier. In grid-based chip design, default value is set to `y * num_cols + x`.
15.  "shape_name" refers to the type of physical probe design. Setting "shape_name" to 0 enables the grid-based chip design. Setting "shape_name" larger than 0 enables the customized shape and region specification
16.  The coordinate system is as same as [10]

### CAD Spec in Details
***

![cad_spec_details.pttx](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/120468193acf21f236482f3da7d1ca96/cad_spec_details.pttx.png)

### Examples
***

Four brief examples are listed here to shows the usage of the CAD in JSON format.

1.  A simple example ([`CPT/example/minimal_example.json`](#minimal-example))

    shows the minimal required information for building a CAD file. Unspecified fields will be set to default values.
    This CAD file defines a __tag-based expression assay__ with the following properties:
    * The probe synthesis is __from 3' to 5'__
    * __Single channel__ is used
    * Only one probeset is defined with an identifier __"HelloCAD"__
    * A probe is designed in dimension __(width = 2, height = 2)__ at location __(x = 10, y = 10)__ 
    * The physical design of probe shape is customized by chip developer

1.  Default usage (example file: [`CPT/example/default_example.json`](#default-usage))

    shows the fully features of CAD file for designing the chip. The designing criteria are as same as `minimal_example.json`.

1.  Tag-based genotyping array (example file: [`CPT/example/genotyping_tag.json`](#tag-based-genotyping))

    shows how to specify the required information of CAD file for tag-based genotyping design.

        position           4  7 10
                           ¦  ¦  ¦  
        chrA+        5-AGAGTTGAGCCTGT
        probe_A            accTcgg
        probe_B            accGcgg
        probeset_name      PS1
        A_allele              A
        B_allele              C

1.  Ligation-based genotyping array (example file: [`CPT/example/genotyping_ligase.json`](#ligation-based-genotyping))

    shows how to specify the required information of CAD file for ligation-based genotyping design.
    The figure shows the designing scenario

        position         2    7       16  20  
                         ¦    ¦        ¦   ¦    
        chrA+        5-ACTGTAGATACTCGTGTCGTACATCGGCGA-3
        probe_AC         acatc>
        probe_AT_1                     agcaT>
        probe_AT_2                     agcaA>
        probeset_name    PS1           PS2 
        A_allele              A            A
        B_allele              C            T

### Minimal Example
***

    {
        "magic": "113",
        "version": "000",
        "probe_array_type":
        {
            "name": "Example",
            "version": "0"
        },
        "num_probesets": "1",
        "num_probes": "1",
        "max_seq_length": "4",
        "num_features": "1",
        "probeset_list": [
        {
                "name": "HelloCAD",
                "type": "Expression",
                "subtype": "TagBased",
                "num_probes": "1",
                "desc": "",
                "probe_list": [
                    {
                        "probe_name": "0",
                        "shape_name": "1",
                        "region_des": [ "10", "10", "2", "2" ]
                    }
                ]
            }
        ]
    }

### Default Usage
***

    {
        "magic": "113",
        "version": "000",
        "probe_array_type":
        {
            "name": "Example",
            "version": "0"
        },
        "num_probesets": "1",
        "num_probes": "1",
        "num_features": "1",
        "num_cols": "0",
        "num_rows": "0",
        "num_channels": "1",
        "max_chn_items": "1",
        "max_seq_length": "4",
        "genome_assembly": "undefined",
        "probe_direction": "3-5",
        "probeset_list": [
        {
                "name": "HelloCAD",
                "type": "Expression",
                "subtype": "TagBased",
                "chrom": "",
                "start": "0",
                "end": "0",
                "strand": ".",
                "desc": "",
                "num_probes": "1",
                "probe_list": [
                    {
                        "probe_name": "0",
                        "shape_name": "1",
                        "region_des": [ "10", "10", "2", "2" ],
                        "channel_des": {
                            "allele": "",
                            "base": "",
                            "channel": ""
                        },
                        "sequence": {
                            "start": "0",
                            "length": "4",
                            "strand": ".",
                            "content": "ACGT"
                        }
                    }
                ]
            }
        ]
    }

### Tag-based Genotyping
***

    {
        "magic": "113",
        "version": "000",
        "probe_array_type":
        {
            "name": "genotyping-array",
            "version": "0"
        },
        "num_probesets": "1",
        "num_probes": "2",
        "num_features": "2",
        "num_cols": "0",
        "num_rows": "0",
        "num_channels": "1",
        "max_chn_items": "2",
        "max_seq_length": "7",
        "genome_assembly": "undefined",
        "probe_direction": "3-5",
        "probeset_list": [
            {
                "name": "PS1",
                "type": "Genotyping",
                "subtype": "TagBased",
                "chrom": "chrA",
                "start": "7",
                "end": "8",
                "strand": "+",
                "desc": "AC",
                "num_probes": "2",
                "probe_list": [
                    {
                        "probe_name": "0",
                        "shape_name": "1",
                        "region_des": [ "10", "10", "2", "2" ],
                        "channel_des": {
                            "allele": "A",
                            "base": "T",
                            "channel": "0"
                        },
                        "sequence": {
                            "start": "4",
                            "length": "7",
                            "strand": "-",
                            "content": "AACTCGG"
                        }
                    },
                    {
                        "probe_name": "1",
                        "shape_name": "1",
                        "region_des": [ "12", "12", "2", "2" ],
                        "channel_des": {
                            "allele": "B",
                            "base": "G",
                            "channel": "0"
                        },
                        "sequence": {
                            "start": "4",
                            "length": "7",
                            "strand": "-",
                            "content": "AACGCGG"
                        }
                    }
                ]
            }
        ]
    }

### Ligation-based Genotyping
***

    {
        "magic": "113",
        "version": "000",
        "probe_array_type":
        {
            "name": "Example",
            "version": "0"
        },
        "num_probesets": "2",
        "num_probes": "3",
        "num_features": "3",
        "num_cols": "0",
        "num_rows": "0",
        "num_channels": "2",
        "max_chn_items": "2",
        "max_seq_length": "5",
        "genome_assembly": "undefined",
        "probe_direction": "3-5",
        "probeset_list": [
            {
                "name": "PS1",
                "type": "Genotyping",
                "subtype": "LigationBased",
                "chrom": "chrA",
                "start": "7",
                "end": "8",
                "strand": "+",
                "desc": "AC",
                "num_probes": "1",
                "probe_list": [
                    {
                        "probe_name": "0",
                        "shape_name": "1",
                        "region_des": [ "10", "10", "2", "2" ],
                        "channel_des": {
                            "allele": "A/B",
                            "base": "T/G",
                            "channel": "1/0"
                        },
                        "sequence": {
                            "start": "2",
                            "length": "5",
                            "strand": "-",
                            "content": "ACATC"
                        }
                    }
                ]
            },
            {
                "name"   : "PS2",
                "type"   : "Genotyping",
                "subtype": "LigationBased",
                "chrom"  : "chrA",
                "start"  : "20",
                "end"    : "21",
                "strand" : "+",
                "desc"   : "AT",
                "num_probes": "2",
                "probe_list": [
                    {
                        "probe_name": "1",
                        "shape_name": "1",
                        "region_des": [ "12", "10", "2", "2" ],
                        "channel_des": {
                            "allele": "A",
                            "base": "T",
                            "channel": "0"
                        },
                        "sequence": {
                            "start": "16",
                            "length": "5",
                            "strand": "-",
                            "content": "AGCAT"
                        }
                    },
                    {
                        "probe_name": "2",
                        "shape_name": "1",
                        "region_des": [ "14", "10", "2", "2" ],
                        "channel_des": {
                            "allele": "B",
                            "base": "A",
                            "channel": "0"
                        },
                        "sequence": {
                            "start": "16",
                            "length": "5",
                            "strand": "-",
                            "content": "AGCAA"
                        }
                    }
                ]
            }
        ]
    }

### HDF5 header for CAD
***

    GROUP "/" {
       GROUP "CAD" {
          ATTRIBUTE "array_type_name" {
             DATATYPE  H5T_STRING {
                STRSIZE H5T_VARIABLE;
                STRPAD H5T_STR_NULLTERM;
                CSET H5T_CSET_ASCII;
                CTYPE H5T_C_S1;
             }
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "array_type_version" {
             DATATYPE  H5T_STRING {
                STRSIZE H5T_VARIABLE;
                STRPAD H5T_STR_NULLTERM;
                CSET H5T_CSET_ASCII;
                CTYPE H5T_C_S1;
             }
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "genome_assembly" {
             DATATYPE  H5T_STRING {
                STRSIZE H5T_VARIABLE;
                STRPAD H5T_STR_NULLTERM;
                CSET H5T_CSET_ASCII;
                CTYPE H5T_C_S1;
             }
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "magic" {
             DATATYPE  H5T_STD_I16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "max_chn_items" {
             DATATYPE  H5T_STD_U16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "max_seq_length" {
             DATATYPE  H5T_STD_U16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "num_channels" {
             DATATYPE  H5T_STD_U16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "num_cols" {
             DATATYPE  H5T_STD_U16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "num_features" {
             DATATYPE  H5T_STD_U32LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "num_probesets" {
             DATATYPE  H5T_STD_U32LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "num_rows" {
             DATATYPE  H5T_STD_U16LE
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "probe_direction" {
             DATATYPE  H5T_STRING {
                STRSIZE H5T_VARIABLE;
                STRPAD H5T_STR_NULLTERM;
                CSET H5T_CSET_ASCII;
                CTYPE H5T_C_S1;
             }
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          ATTRIBUTE "version" {
             DATATYPE  H5T_STRING {
                STRSIZE 3;
                STRPAD H5T_STR_NULLTERM;
                CSET H5T_CSET_ASCII;
                CTYPE H5T_C_S1;
             }
             DATASPACE  SIMPLE { ( 1 ) / ( 1 ) }
          }
          DATASET "PROBESET_LIST" {
             DATATYPE  H5T_COMPOUND {
                H5T_STRING {
                   STRSIZE 64;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "name";
                H5T_STRING {
                   STRSIZE 32;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "type";
                H5T_STRING {
                   STRSIZE 32;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "subtype";
                H5T_STRING {
                   STRSIZE 8;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "chrom";
                H5T_STD_U32LE "start";
                H5T_STD_U32LE "end";
                H5T_STRING {
                   STRSIZE 4;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "desc";
                H5T_STRING {
                   STRSIZE 1;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "strand";
                H5T_STD_U16LE "num_probes";
                H5T_REFERENCE { H5T_STD_REF_DSETREG } "probes_ref";
             }
          }
          DATASET "PROBE_LIST" {
             DATATYPE  H5T_COMPOUND {
                H5T_STD_U32LE "probe_name";
                H5T_STD_U16LE "shape_name";
                H5T_STD_U32LE "x";
                H5T_STD_U32LE "y";
                H5T_STD_U16LE "w";
                H5T_STD_U16LE "h";
                H5T_STD_U32LE "start";
                H5T_STD_U16LE "length";
                H5T_STRING {
                   STRSIZE 1;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "strand";
                H5T_STRING {
                   STRSIZE 8;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "allele";
                H5T_STRING {
                   STRSIZE 8;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "base";
                H5T_STRING {
                   STRSIZE 8;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "channel";
                H5T_STRING {
                   STRSIZE 128;
                   STRPAD H5T_STR_NULLTERM;
                   CSET H5T_CSET_ASCII;
                   CTYPE H5T_C_S1;
                } "sequence";
             }
          }
       }
    }
