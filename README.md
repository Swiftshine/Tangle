# Tangle
An (experimental) tool for [GFA (GoodFeelArchive) Files](https://swiftshine.github.io/documentation/gfa.html).

## Special Thanks
[BPE de/compression source](http://www.pennelynn.com/Documents/CUJ/HTML/94HTML/19940045.HTM)

[LZ77 de/compression source](https://github.com/Favrito/LZ77/blob/master/main.c)

## Usage
`tangle <usage> <input> <output> <compression type>`

| usage keyword | meaning |
|-|-|
|archive|creates a gfa file and compresses the contents with the specified compression type|
|unpack|unpacks a gfa file into the `output` folder. only the input filename is needed |

`input` - the input filename

`output` - the output filename, if applicable

`compression type` - can be one of the following:
| compression type | usage |
|-|-|
|bpe | Byte Pair Encoding. <em>Used in Kirby's Epic Yarn</em> and <em>Yoshi's Woolly World</em>.|
|lz77| LZ77. Used in <em>Yoshi and Poochy's Woolly World</em> and <em>Kirby's Extra Epic Yarn</em>.
| none | no compression |


If you are unpacking a file, only the input is specified.

## Changelog
`1st December 2023` - v0.1

`11 May 2023` - Rewrite, v1.0, v1.1
