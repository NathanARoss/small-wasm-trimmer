# small-wasm-trimmer
This unility specifically removes custom sections from .wasm binaries.

I needed a tool to remove user generated sections from .wasm binaries produced by compilers.  All sections except user defined sections are copied from stdin to stdout.  Errors are reported on stderr.

I would like to expand this tool to accept command-line arguments to allow preserving the "name" user-defined section and to allow removing specific exports.  This tool is specifically designed for trimming down .wasm binaries in straightforward ways that compilers do not automatically.
