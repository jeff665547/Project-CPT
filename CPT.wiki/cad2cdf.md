cad2cdf
===
## Description

This program converts the Centrillion CAD file (.cad, .h5, .hdf5) into Affymetrix CDF file ( XDA format ).

### The atom and cells in CDF

Because there is no *atom* or *cell* concept in CAD, so the conversion result assume one cell, one probe and one cell per atom.

### The tbase and pbase in CDF

Affymetrix CDF use *tbase* and *pbase* to detect PM ( perfect match ) and MM ( mismatch ) event, but there is some research had identified that *the MM signal may higher than PM significantly* so in CAD file format, **we remove the *tbase* and *pbase***.


Because the tbase and pbase are absence in CAD but the content of these fields are necessary for CDF, so the converter is designed to **use the last base of the probe as pbase and the complementary of pbase as tbase**.

## Usage

```shell=
cad2cdf
Allowed options:
  -h [ --help ]         show help message
  -i [ --input ] arg    CAD file (.cad, .h5, .hdf5 )
  -o [ --output ] arg   CDFfile (.cdf)
  
cad2cdf -i input.cad -o output.cdf
```

## Bioconductor Compatible

### The spec conflict of unit type 

The UnitType in CDF is an integer id mapping to type field, and we found in [Affymetrix CDF spec description](
http://www.affymetrix.com/support/developer/powertools/changelog/gcos-agcc/cdf.html), the mapping rule is:

1 - Expression

2 - Genotyping 

3 - CustomSeq

etc.

But in Bioconductor the mapping rule is different:

0 - Expression

1 - Genotyping

2 - Tag

etc.

To compatible with Bioconductor, **the unit type conversion rule in ```cad2cdf``` is designed to follow the Bioconductor**



