anagram
=======

Messing about with anagram hashes and other code.

The idea, here, is to take words and apply a hash function where all anagrams
produce the same hash; but to take this concept further by attempting, within
`n` bits, to compress the information in a lossless way, so that there are no
false positives.

Since this is clearly not possible for arbitrary input on a finite-sized hash,
we can use one bit to indicate whether or not the hash is perfect, and
otherwise to descend into more hash-like behaviour.  This gives fast-out cases
for non-matches and most (practically all!) matches alike.

Unfortunately it looks like I neglected to check in the scripts I used to
analyse the output.
