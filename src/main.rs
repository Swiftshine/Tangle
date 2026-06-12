use anyhow::Result;
use clap::{Parser, ValueEnum, Subcommand};
use crossterm::style::Stylize;
use gfarch;
use std::num::ParseIntError;
use std::fs;
use std::path::PathBuf;

/* Options */

#[derive(Debug, Clone, ValueEnum)]
enum Version {
    #[value(name = "2.0", aliases = ["2"])]
    V2,
    #[value(name = "3.0", aliases = ["3"])]
    V3,
    #[value(name = "3.1")]
    V3_1,
}

#[derive(Debug, Clone, ValueEnum)]
enum CompressionType {
    None,
    BPE,
    LZ10,
    Zlib,
}

/* Command-line */

#[derive(Subcommand)]
enum Command {
    Extract {
        input: PathBuf,
        output: Option<PathBuf>,
    },
    
    Archive {
        input: PathBuf,
        output: PathBuf,
        version: Version,
        compression_type: CompressionType,

        #[arg(short = 'o', long, value_parser = parse_hex)]
        gfcp_offset: Option<usize>,

        #[arg(short, long, value_parser = parse_hex)]
        alignment: Option<usize>,
    },
}

#[derive(Parser)]
#[command(name = "tangle")]
struct Cli {
    #[command(subcommand)]
    command: Command,
}

fn parse_hex(s: &str) -> Result<usize, ParseIntError> {
    if s.starts_with("0x") || s.starts_with("0X") {
        u64::from_str_radix(&s[2..], 16).map(|x| x as usize)
    } else {
        s.parse::<usize>()
    }
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    match cli.command {
        Command::Extract { input, output } => {
            // extract file contents
            let archive = fs::read(&input)?;
            let extracted = gfarch::extract(&archive)?;

            // create folder
            let folder_name = output
                .clone()
                .unwrap_or_else(|| input.with_extension(""));

            if !folder_name.exists() {
                fs::create_dir_all(&folder_name)?;
            }

            // write files
            let mut num_errors = 0;
            let num_files = extracted.len();

            for file in extracted {
                let result = fs::write(folder_name.join(&file.0), file.1);

                if let Err(e) = result {
                    eprintln!(
                        "{} \"{}\" {}. Error: {}",
                        "Failed to write the contents of".yellow(),
                        file.0,
                        "to disk.".yellow(),
                        e
                    );
                    num_errors += 1;
                }
            }

            if num_errors != 0 {
                if num_errors != num_files {
                    println!(
                        "{} {} {}",
                        "Finished with".yellow(),
                        num_errors,
                        "errors.".yellow()
                    );
                } else {
                    println!("{}", "Failed.".red());
                    std::process::exit(1);
                }
            }
        }

        Command::Archive { input, output, version, compression_type, gfcp_offset, alignment } => {
            if !input.is_dir() {
                eprintln!("{}", "Input must be a folder.".red());
                std::process::exit(1);
            }

            fn is_dir_empty(path: &PathBuf) -> Result<bool> {
                Ok(fs::read_dir(path)?.next().is_none())
            }
            
            if is_dir_empty(&input)? {
                eprintln!(
                    "{} \"{}\" {}",
                    "The folder".red(),
                    input.display(),
                    "is empty.".red()
                );
                std::process::exit(1);
            }

            let mut files: Vec<(String, Vec<u8>)> = Vec::new();

            let mut num_errors = 0;
            
            for entry in fs::read_dir(input)? {
                let entry = if entry.is_ok() {
                    entry.unwrap()
                } else {
                    num_errors += 1;
                    continue;
                };

                let path = entry.path();
                let filename = path.file_name().unwrap().to_string_lossy().to_string();
                let contents = fs::read(path)?;

                files.push((filename, contents));
            }
            
            // determine parameters
            let compression_type = match compression_type {
                CompressionType::None => gfarch::CompressionType::None,
                CompressionType::BPE => gfarch::CompressionType::BPE,
                CompressionType::LZ10 => gfarch::CompressionType::LZ10,

                CompressionType::Zlib => gfarch::CompressionType::Zlib,
            };

            let version = match version {
                Version::V2 => gfarch::Version::V2,
                Version::V3 => gfarch::Version::V3,
                Version::V3_1 => gfarch::Version::V3_1,
            };

            let offset = match gfcp_offset {
                Some(offs) => gfarch::GFCPOffset::Custom(offs),
                None => gfarch::GFCPOffset::Default
            };

            let alignment = match alignment {
                Some(align) => gfarch::Alignment::Custom(align),
                None => gfarch::Alignment::Default
            };

            // create archive
            let archive = gfarch::pack_from_files(
                &files,
                version,
                compression_type,
                offset,
                alignment
            );

            // write archive
            let output_path = if output.extension().is_some() {
                output.clone()
            } else {
                output.with_extension("gfa")
            };

            fs::write(output_path, archive)?;

            if num_errors != 0 {
                println!(
                    "{} {} {}",
                    "Failed to store".red(),
                    num_errors,
                    "files in the archive.".red()
                );
            }
        }
    }

    println!("{}", "Done.".green());
    Ok(())
}
