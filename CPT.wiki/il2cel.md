il2cel
===
## Description
This program converts the ```intensities_dump``` result ( intensities list ) into Affymetrix CEL file.

Because the intensities list is a TSV file and only intensities mean are in the file, so the converted result CEL file has only intensities mean.

## Usage
```
$il2cel --help
Allowed options:
  -h [ --help ]         show help message
  -i [ --input ] arg    intensity list file (.tsv)
  -o [ --output ] arg   cel file (.cel)

$il2cel -i intensity.tsv -o result.cel
```

## Bioconductor compatible

There are 2 known limitations in Bioconductor's "**affxpaser**" library: 
1. Multi-channel CEL file is not supported.
2. The data group name cannot change.

Because of compatibility, ```il2cel``` is designed to follow these limitations.