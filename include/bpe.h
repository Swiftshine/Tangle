#pragma once
void expand(FILE *input, FILE *output);

namespace BPE {
    void decompress(std::fstream&, std::fstream&);
    void compress(FILE* input, FILE* output);


} // namespace BPE