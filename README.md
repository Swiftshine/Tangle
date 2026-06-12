# Tangle
A Good-Feel archive tool.
Documentation on the format can be found [here](https://swiftshine.github.io/doc/gfa.html).

## Usage
Use `tangle -h` or `tangle --help` to get a list of needed arguments.

### Extracting an archive
```
tangle extract my_file.gfa
```
or
```
tangle extract my_file.gfa my_folder
```

### Creating an archive
Good-Feel archives are created with Byte Pair Encoding (`bpe`), LZ10 (`lz10`), or no compression at all (leave blank, or `none`).

Here is an example of a GfArch v3 archive being created:
```
tangle archive my_folder my_file.gfa 3.0 bpe
```

The same example, but for GfArch v3.1:
```
tangle archive my_folder my_file.gfa 3.1 bpe
```

If you are targeting a specific game, the version and compression types are as follows:

| Version | Game | Compression | Tangle arg |
| - | - | - | - |
| 2.0 | <em>Wario Land: Shake It!</em> | Byte Pair Encoding | `2` or `2.0` | 
| 3.0 | <em>Kirby's Epic Yarn</em> | Byte Pair Encoding | `3` or `3.0` |
| 3.1 | <em>Yoshi's Woolly World | Byte Pair Encoding | `3.1`|
| 3.1 | <em>Poochy & Yoshi's Woolly World | LZ10 | `3.1` |
| 3.1 | <em>Kirby's Extra Epic Yarn</em> | LZ10 | `3.1` |

### Custom compression
Here is a list of supported custom compression types:

| Compression type | Tangle arg |
| - | - |
| zlib | `zlib` |

#### Todo - additional GfArch confirmations
Check the following:
- Looksley's Line Up (version: 3.0, compression: unknown)
- Mii Plaza (version: 3.1? compression: bpe?)
- Other Good-Feel titles that use GfArch
