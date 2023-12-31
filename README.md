# Tangle
An (experimental) tool for [GFA (GoodFeelArchive) Files](https://swiftshine.github.io/documentation/gfa.html).

## Special Thanks
[BPE de/compression source](http://www.pennelynn.com/Documents/CUJ/HTML/94HTML/19940045.HTM)

[LZ77 de/compression source](https://github.com/Favrito/LZ77/blob/master/main.c)

## Usage
### Command Syntax
`tangle [1] <input name> <output name> [2] [3]`
### Parameters
In most cases, you'd only want to specify `[1]`, `<input name>`, `<output name>`, and `[2]`.


`[1]`: Required. Specifies what the program is meant to do.
* `-c` to pack the contents of a folder into a .gfa archive.
* `-d` to unpack the contents of a .gfa archive into a folder.

`<input name>`: Required. The name of the archive or folder to manipulate.

`<output name>`: Optional. The name of the archive or folder to be output. If this is not specified, the output will be `<input name>.gfa` (archive) or `<input name> - extracted` (folder).

`[2]` Optional. This option is used to specify which game to make a .gfa archive for. The options are as follows:

	0. Default
	1. Kirby's Epic Yarn (Wii)
	2. Yoshi's Wooly World (Wii U)
	3. Poochy and Yoshi's Wooly World (3DS)
	4. Kirby's Extra Epic Yarn (3DS)

If any of the below conditions are met, this value will be ignored:
* `[1]` is `-d`
* The value of `[2]` is not any of the above options

**Note** - if `[2]` is 0, it will use the same type of compression as 1 and 2 (Byte Pair Encoding). (If you want to use LZ77, you can set `[2]` to 4, as there is current no (known) offset specification)

`[3]`: Optional. This option is used to specify which offset the GFCP Header should be located at. If any of the below conditions are met, this option will be ignored:
* `[1]` is `-d`
* `[2]` is a valid option (you may want to set this to 0 if you want to use `[3]`)
* `[3]` is negative
* `[3]` is larger than a specific size determined by the number of files and the length of their filenames (the program will tell you)
* `[3]` is not a multiple of 16 (0x10)

This is for the sake of consistency.

This value can be input as decimal or hex. To input as hex, just append `0x` to the beginning.
## Progress
- [X] archive manipulation
	- [X] GFA::unpack() structure
	- [X] GFA::unpack() functionality
	- [X] GFA::pack() structure
	- [X] GFA::pack() functionality
- [X] compression
	- [X] BPE compression
	- [X] BPE decompression
	- [X] LZ77 compression
	- [X] LZ77 decompression
- [ ] cleanup
- [ ] finer details
	- [ ] figure out usage for archive header offset 0x4
	- [ ] figure out how file entry hashes are calculated
## Changelog
`1st December 2023` - v1.0


## Comments
Honestly this just ended up being something I started working on because there weren't any tools that could actually do anything with this, so I decided to make my own.

Ultimately I think the way I made it was fine, though there are definitely some ugly parts that I'd like to spend more time improving but can't.

Are the packed archives guaranteed to work in game? I don't think so (but I think they should work).
