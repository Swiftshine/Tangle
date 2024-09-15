# Tangle
A GoodFeelArchive tool.

## Special Thanks
- [BPE decoding source](http://www.pennelynn.com/Documents/CUJ/HTML/94HTML/19940045.HTM)
- [BPE encoding source](https://web.archive.org/web/20160807201159/https://www.csse.monash.edu.au/cluster/RJK/Compress/bpe.c)
- [LZ77 de/compression source](https://github.com/Favrito/LZ77/blob/master/main.c)

## Usage
There are multiple revisions of the GfArch format.
| Version | Game | Compression |
| - | - | - |
| 2.0 | <em>Wario Land: Shake It!</em> | Byte Pair Encoding |
| 3.0 | <em>Kirby's Epic Yarn</em> | Byte Pair Encoding |
| 3.1a | <em>Yoshi's Woolly World | Byte Pair Encoding |
| 3.1b | <em>Poochy & Yoshi's Woolly World | LZ77 |
| 3.1b | <em>Kirby's Extra Epic Yarn</em> | LZ77 |

Before creating an archive, the user must specify the archive version by typing `2.0`, `3.0`, `3.1a`, or `3.1b`.

Operations involving LZ77 are by no means guaranteed to work.

### Todo - additional GfArch confirmations
Check the following:
- Looksley's Line Up (version: 3.0, compression: unknown)
- Mii Plaza (version: 3.1? compression: bpe?)
- Other Good-Feel titles that use GfArch

## Building
This project uses [portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs). Be sure to include any necessary libraries for the file dialogs. For example, Windows requires `ole32`, `comctl32`, `oleaut32`, and `uuid`.
