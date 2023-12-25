```
{
    "A|magic": {                                        # CEN file signature
        "DATATYPE": "uint8_t",
        "DATA": 67
    },
    "A|version": {                                      # CEN file version
        "DATATYPE": "uint8_t",
        "DATA": 1
    },
    "A|status": {                                       # used to indicate the status of CEN file
        "DATATYPE": "std::string",
        "DATA": "uninitialized"
    },
    "G|array": {                                        # Group by probe array specific data
        "A|date": {                                     # Manufacturing date
            "DATATYPE": "std::string",
            "DATA": "YYYY-MM-DD HH:MM:SS"
        },
        "A|type": {                                     # Probe array type
            "DATATYPE": "std::string",
            "DATA": "centrillion-gene-chip"
        },
        "A|barcode": {                                  # Barcode
            "DATATYPE": "std::string",
            "DATA": "abcde-0123456789"
        },
        "A|feature-columns": {                          # Total number of feature columns
            "DATATYPE": "uint16_t",
            "DATA": 8
        },
        "A|feature-rows": {                             # Total number of feature rows
            "DATATYPE": "uint16_t",
            "DATA": 8
        },
        "A|feature-height": {                           # Average height of a feature in micrometers
            "DATATYPE": "uint16_t",
            "DATA": 5
        },
        "A|feature-width": {                            # Average width of a feature in micrometers
            "DATATYPE": "uint16_t",
            "DATA": 5
        },
        "G|channel-0": {                                # Group by the wave length of measurement, please refer to "/scanner/channel-0/wave-length" in detail
            "D|intensity": {                            # Signal intensities of features. The array index denotes the probe or feature ID.
                "MEMBERS"  : ["intensity"],
                "DATATYPE" : ["float"],
                "DATASPACE": [64],
                "DATA": [
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ]
                ]
            },
            "D|stddev": {                               # Standard deviations of signal intensity extracted from the image processing of raw image
                "MEMBERS"  : ["stddev"],
                "DATATYPE" : ["float"],
                "DATASPACE": [64],
                "DATA": [
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ]
                ]
            },
            "D|pixel": {                                # A list of the number of pixels for calculating a probe intensity
                "MEMBERS"  : ["pixel"],
                "DATATYPE" : ["int16_t"],
                "DATASPACE": [64],
                "DATA": [
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ]
                ]
            },
            "D|mask": {                                 # A list of probe xy coordinates masked out by user
                "MEMBERS"  : ["x"      , "y"      ],
                "DATATYPE" : ["int16_t", "int16_t"],
                "DATASPACE": [1],
                "DATA": [
                    [0, 0]
                ]
            },
            "D|outlier": {                              # A list of probe xy coordinates removed by image processing algorithm
                "MEMBERS"  : ["x"      , "y"      ],
                "DATATYPE" : ["int16_t", "int16_t"],
                "DATASPACE": [5],
                "DATA": [
                    [1, 1],
                    [2, 2],
                    [3, 3],
                    [4, 4],
                    [5, 5]
                ]
            }
        },
        "G|channel-1": {                                # Group by the wave length of measurement, please refer to "/scanner/channel-1/wave-length" in detail
            "D|intensity": {                            # Signal intensities of features. The array index denotes the probe or feature ID.
                "MEMBERS"  : ["intensity"],
                "DATATYPE" : ["float"],
                "DATASPACE": [64],
                "DATA": [
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ],
                    [ 0 ], [ 0 ], [ 0 ], [ 0 ]
                ]
            },
            "D|stddev": {                               # Standard deviations of signal intensity extracted from the image processing of raw image
                "MEMBERS"  : ["stddev"],
                "DATATYPE" : ["float"],
                "DATASPACE": [64],
                "DATA": [
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ],
                    [ 1 ], [ 1 ], [ 1 ], [ 1 ]
                ]
            },
            "D|pixel": {                                # A list of the number of pixels for calculating a probe intensity
                "MEMBERS"  : ["pixel"],
                "DATATYPE" : ["int16_t"],
                "DATASPACE": [64],
                "DATA": [
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ],
                    [ 9 ], [ 9 ], [ 9 ], [ 9 ]
                ]
            },
            "D|mask": {                                 # A list of probe xy coordinates masked out by user
                "MEMBERS"  : ["x"      , "y"      ],
                "DATATYPE" : ["int16_t", "int16_t"],
                "DATASPACE": [1],
                "DATA": [
                    [0, 0]
                ]
            },
            "D|outlier": {                              # A list of probe xy coordinates removed by image processing algorithm
                "MEMBERS"  : ["x"      , "y"      ],
                "DATATYPE" : ["int16_t", "int16_t"],
                "DATASPACE": [5],
                "DATA": [
                    [1, 1],
                    [2, 2],
                    [3, 3],
                    [4, 4],
                    [5, 5]
                ]
            }
        }
    },
    "G|image-processing": {                             # Group by image specific processing stage
        "G|algorithm": {                                # Group by image processing algorithm. The schema would be depended on the use of algorithm
            "A|name": {                                 # Name of algorithm
                "DATATYPE": "std::string",
                "DATA": "algorithm-name"
            },
            "A|version": {                              # Version of algorithm
                "DATATYPE": "std::string",
                "DATA": "algorithm-version"
            }
        },
        "G|channel-0": {                                # Group by the wave length of measurement, please refer to node "/scanner/channel-0/wave-length" in detail
            "I|image": {                                # The processed result image of channel-0
                "PATH": "/path/to/image"                # Path to the image in local file system
            }
        },
        "G|channel-1": {                                # Group by the wave length of measurement, please refer to node "/scanner/channel-1/wave-length" in detail
            "I|image": {                                # The processed result image of channel-1
                "PATH": "/path/to/image"                # Path to the image in local file system
            }
        }
    },
    "G|scanner": {                                      # Group by scanner specific data
        "A|date": {                                     # Recording date
            "DATATYPE": "std::string",
            "DATA": "YYYY-MM-DD HH:MM:SS"
        },
        "A|type": {                                     # Scanner type
            "DATATYPE": "std::string",
            "DATA": "example"
        },
        "G|channel-0": {                                # Details about channel-0
            "A|name": {                                 # Name of channel-0, e.g. "CG"
                "DATATYPE": "std::string",
                "DATA": "CG"
            },
            "A|wave-length": {                          # Wave length of channel-0
                "DATATYPE": "uint32_t",
                "DATA": 531
            },
            "A|exposure-time-ms": {                     # Exposure time in milliseconds
                "DATATYPE": "float",
                "DATA": 200.0
            },
            "I|image": {                                # A raw image shoot by camera
                "PATH": "/path/to/image"                # Path to the image in local file system
            }
        },
        "G|channel-1": {                                # Details about channel-1
            "A|name": {                                 # Name of channel-1, e.g. "AT"
                "DATATYPE": "std::string",
                "DATA": "AT"
            },
            "A|wave-length": {                          # Wave length of channel-1
                "DATATYPE": "uint32_t",
                "DATA": 609
            },
            "A|exposure-time-ms": {                     # Exposure time in milliseconds
                "DATATYPE": "float",
                "DATA": 200.0
            },
            "I|image": {                                # A raw image shoot by camera
                "PATH": "/path/to/image"                # Path to the image in local file system
            }
        }
    },
    "G|protocol": {                                     # Group by protocol-related stages. Currently, this group is unused.
        "G|amplification": {                            # Group by amplification stage
        },
        "G|fragmentation": {                            # Group by fragmentation stage
            "I|image": {
                "PATH": "/path/to/image"                # Path to the image in local file system
            }
        },
        "G|hybridization": {                            # Group by hybridization stage
            "A|date": {                                 # Recording date
                "DATATYPE": "std::string",
                "DATA": "YYYY-MM-DD HH:MM:SS"
            },
            "A|start-time": {                           # The time of starting a hybridization process
                "DATATYPE": "std::string",
                "DATA": "YYYY-MM-DD HH:MM:SS"
            },
            "A|stop-time": {                            # The time of Stopping time of hybridization processes
                "DATATYPE": "std::string",
                "DATA": "YYYY-MM-DD HH:MM:SS"
            },
            "A|set-temperature": {                      # the setting of hybridization temperature
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|max-temperature": {                      # max temperature during hybridization process
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|min-temperature": {                      # min temperature during hybridization process
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|concentration-of-sodium": {              # concentration of sodium
                "DATATYPE": "float",
                "DATA": 1.0
            },
            "A|concentration-of-magnesium": {           # concentration of magnesium
                "DATATYPE": "float",
                "DATA": 0.0
            },
            "A|concentration-of-dna": {                 # concentration of the DNA sample
                "DATATYPE": "float",
                "DATA": 100.0
            }
        },
        "G|fluidics": {                                 # Group by fluidic device-related stage
            "A|date": {                                 # Recording date
                "DATATYPE": "std::string",
                "DATA": "YYYY-MM-DD HH:MM:SS"
            },
            "A|type": {                                 # Device type
                "DATATYPE": "std::string",
                "DATA": "device"
            },
            "A|duration": {                             # Duration of processing
                "DATATYPE": "std::string",
                "DATA": "hh:mm:ss"
            },
            "A|wash-cycle": {                           # Number of washing cycle
                "DATATYPE": "uint32_t",
                "DATA": 1
            },
            "A|wash-set-temperature": {                 # the setting of wash temperature
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|wash-max-temperature": {                 # max temperature during washing process
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|wash-min-temperature": {                 # min temperature during washing process
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|water-wash-set-temperature": {           # the setting of water wash temperature
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|water-wash-max-temperature": {           # max temperature of water washing process
                "DATATYPE": "float",
                "DATA": 36.0
            },
            "A|water-wash-min-temperature": {           # min temperature of water washing process
                "DATATYPE": "float",
                "DATA": 36.0
            }
        },
        "G|user-defined-parameters": {                  # User defined parameters
        }
    },
    "G|legacy": {                                       # A collection of unrelated items when we convert a CEL to CEN file
    }
}
```