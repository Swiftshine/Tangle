use clap::{Parser, ValueEnum};
use gfarch::gfarch;
use anyhow::Result;
use std::{fs, path::Path};
use colored::*;
use std::num::ParseIntError;

#[derive(Debug, Clone, ValueEnum)]
enum ArchiveVersion {
    V2,
    V3,
    V3_1,
    // V3_1a,
    // V3_1b, // not supported yet
}

#[derive(Debug, Clone, ValueEnum)]
enum ToolUsage {
    Extract, // To extract an archive's contents
    Archive // To create an archive
}

#[derive(Debug, Clone, ValueEnum)]
enum CompressionType {
    BPE,
    LZ10
}

#[derive(Parser, Debug)]
#[command(name = "tangle")]
struct Args {
    /// Tool usage.
    usage: ToolUsage,
    /// The input file or folder.
    input: String,
    /// The output file or folder. Required when creating an archive.
    output: Option<String>,
    /// The version to be used when creating an archive.
    archive_version: Option<ArchiveVersion>,
    /// The compression to be used when creating an archive.
    compression_type: Option<CompressionType>,
    /// The GFCP offset to be used, if needed. Hexadecimal format is supported.
    gfcp_offset: Option<String>,
}

fn file_to_folder(file_name: &str) -> String {
    let path = Path::new(file_name);
    match path.file_stem() {
        Some(stem) => stem.to_string_lossy().to_string(),
        None => file_name.to_string(), // fallback if there's no stem
    }
}

fn parse_offset(s: &str) -> Result<usize, ParseIntError> {
    if s.starts_with("0x") || s.starts_with("0X") {
        u64::from_str_radix(&s[2..], 16).map(|x| x as usize)
    } else {
        s.parse::<usize>()
    }
}

fn main() -> Result<()> {
    let args = Args::parse();

    match args.usage {
        ToolUsage::Extract => {
            // extract file contents
            let archive = fs::read(&args.input)?;
            let extracted = gfarch::extract(&archive)?;
            
            let folder_name = if args.output.is_some() {
                &args.output.unwrap()
            } else {
                &args.input
            };

            // create folder
            let folder_name = file_to_folder(&folder_name) + "/";
            if !fs::exists(&folder_name)? {
                fs::create_dir(&folder_name)?;
            }

            // write files
            let mut num_errors = 0usize;
            let num_files = extracted.len();
            for file in extracted {
                let result = fs::write(folder_name.to_string() + &file.0, file.1);
                if let Err(e) = result {
                    eprintln!("{} '{}' {}. Error: {}", "Failed to write the contents of".yellow(), file.0, "to disk.".yellow(), e);
                    num_errors += 1;
                }
            }

            if num_errors != 0 {
                if num_errors != num_files {
                    println!("{} {} {}", "Finished with".yellow(), num_errors, "errors.".yellow());
                } else {
                    println!("{}", "Failed.".red());
                    std::process::exit(1);
                }
            }
        },

        ToolUsage::Archive => {
            // validate input
            if !fs::exists(&args.input)? {
                eprintln!("{} '{}' {}", "The folder".red(), args.input, "does not exist.".red());
                std::process::exit(1);
            }
            
            
            if args.archive_version.is_none() {
                eprintln!("{}", "An archive version is required.".red());
                std::process::exit(1);
            }
            
            let archive_version = args.archive_version.unwrap();

            if args.compression_type.is_none() {
                eprintln!("{}", "A compression type is required.".red());
                std::process::exit(1);
            }
            
            let compression_type = args.compression_type.unwrap();
            
            // read file contents
            let mut files: Vec<(String, Vec<u8>)> = Vec::new();
            
            let mut num_errors = 0;
            for entry in fs::read_dir(&args.input)? {
                let entry = if entry.is_ok() { entry.unwrap() } else {
                    num_errors += 1;
                    continue; 
                };
                
                let path = entry.path();
                let filename = path.file_name().unwrap_or_default().to_string_lossy().to_string();
                let contents = fs::read(path)?;
                
                files.push((filename, contents));
            }
            
            if files.len() == 0 {
                eprintln!("{} '{}' {}", "The folder".red(), &args.input, "is empty.".red());
                std::process::exit(1);
            }

            // convert to something for the gfarch crate to use
            let compression_type = match compression_type {
                CompressionType::BPE => gfarch::CompressionType::BPE,
                CompressionType::LZ10 => gfarch::CompressionType::LZ10,
            };
            
            let archive_version = match archive_version {
                ArchiveVersion::V2 =>   gfarch::Version::V2,
                ArchiveVersion::V3 =>   gfarch::Version::V3,
                ArchiveVersion::V3_1 => gfarch::Version::V3_1,
            };

            let offset = match args.gfcp_offset {
                Some(o) if parse_offset(&o).is_ok() => {
                    gfarch::GFCPOffset::Custom(parse_offset(&o).unwrap())
                }
                _ => gfarch::GFCPOffset::Default,
            };

            // create archive
            let archive = gfarch::pack_from_files(
                &files,
                archive_version,
                compression_type,
                offset
            );
            
            // write archive
            
            let output_name = match args.output {
                Some(ref name) => {
                    // if the name ends with ".gfa", remove it
                    if name.ends_with(".gfa") {
                        &name[..name.len() - 4]  // remove the last 4 characters
                    } else {
                        name
                    }
                }
                None => &args.input,
            }.to_string();

            
            fs::write(output_name + ".gfa", archive)?;

            if num_errors != 0 {
                println!("{} {} {}", "Failed to store".red(), num_errors, "in the archive.".red());
            }
        }
    }
    
    println!("{}", "Done.".green());
    Ok(())
}
