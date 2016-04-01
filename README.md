# wnaabi
Who Needs an ABI?

This project presents a nifty implementation of `type_index`
with some features you will love:

- Canonical typename format consistent across all compilers
- Access to typename **tokens** so you can hate on my canonical
  format however you want.
- Efficient creation, hashing, and typename retrieval- most of the
  work is done at compile time and this project doesn't even contain
  the keyword `virtual`.

This is part of a larger project which isn't ready for prime time yet.
The premise is that we don't really need an [ABI](https://isocpp.org/files/papers/n4028.pdf).
The combined power of C++'s type system and `constexpr` functions
enable us to do anything we would use an ABI for better than we'd do it
with the ABI.
