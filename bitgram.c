#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned long long hash_t;
#define HASH_OVERFLOW (hash_t)(1ull << (sizeof(hash_t) * 8 - 1))

static const uint8_t freqorder[256] = {/*{{{*/
     101, 115, 105,  97, 110, 114, 116, 111, 108,  99, 100, 117, 103, 112, 109, 104,  98, 121, 102, 118, 107, 119, 120, 122, 106, 113,  39,  45,  96, 123, 124, 125,
      48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  32,  33,  34,  35,  36,  37,  38,  40,  41,  42,  43,  44,  46,  47,  58,  59,  60,  61,  62,  63, 126, 127,
     224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
     160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
      69,  83,  73,  65,  78,  82,  84,  79,  76,  67,  68,  85,  71,  80,  77,  72,  66,  89,  70,  86,  75,  87,  88,  90,  74,  81,  64,  91,  92,  93,  94,  95,
     192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
     128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
/*}}}*/};
static const uint8_t alphaorder[256] = {/*{{{*/
     97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  39,  45,  96, 123, 124, 125,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  32,  33,  34,  35,  36,  37,  38,  40,  41,  42,  43,  44,  46,  47,  58,  59,  60,  61,  62,  63, 126, 127,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
     65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  64,  91,  92,  93,  94,  95,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
      1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
/*}}}*/};

static const uint8_t *order = freqorder;

typedef struct
{
    uint64_t code;
    int codelen;
} huffnode_t;

huffnode_t first_huff[256];
huffnode_t delta_huff[256];
huffnode_t count_huff[256];

unsigned int first_freq[256];
unsigned int delta_freq[256];
unsigned int count_freq[256];

unsigned short length_freq[256];

int huffman = 0;
int analyse = 0;
uint64_t cap = 0;

static int enc_first(uint64_t *out, int ch)
{
    if (huffman)
    {
        *out = first_huff[ch].code;
        return first_huff[ch].codelen;
    }
    *out = 0;
    return ch;
}

static int enc_delta(uint64_t *out, int ch)
{
    if (huffman)
    {
        *out = delta_huff[ch].code;
        return delta_huff[ch].codelen;
    }
    *out = 0;
    return ch;
}

static int enc_count(uint64_t *out, int ct)
{
    if (huffman)
    {
        *out = count_huff[ct].code;
        return count_huff[ct].codelen;
    }
    *out = (1ull << ct) - 1;
    return ct;
}

static int mix(hash_t *hash, int *hashbits, uint64_t bits, int bitcount)
{
    int outsize = sizeof(*hash) * 8 - 1;
    int overflow = 0;
    hash_t out = *hash;
    int outbits = *hashbits;
    do
    {
        out ^= bits << outbits;
        outbits += bitcount;
        if (outbits <= outsize)
        {
            *hash = out;
            *hashbits = outbits;
            return overflow;
        }

        /* this part is probably full of bugs */
        out &= ~HASH_OVERFLOW;
        out ^= out >> 17;
        out *= 0xc2b2ae35;
        bits >>= outsize - (outbits - bitcount);
        bitcount -= outsize - (outbits - bitcount);
        outbits = 0;
        overflow++;
    } while (1);
}

static hash_t hash(char const *s)
{
    unsigned short hist[256] = { 0 };
    int total = 0;
    uint8_t const *o = order;
    int overflow = 0;
    hash_t out = 0;
    int outbits = 0;
    int c;
    int d = 0;
    int p = -1;


    while ((c = (tolower(*s) & 0xff)) != '\0')
    {
        hist[c]++;
        total++;
        s++;
    }

    while (total > 0)
    {
        uint64_t bits;
        int bitcount;
        c = 0;

        while ((c = hist[*o++]) == 0 && d < 63)
            d++;
 
        if (analyse)
        {
            if (p < 0)
                first_freq[d]++;
            else
                delta_freq[d]++;
            count_freq[c]++;
        }
        else
        {
            if (p < 0)
                bitcount = enc_first(&bits, d);
            else /* TODO: could use different tables for different stages of the alphabet */
                bitcount = enc_delta(&bits, d);
            overflow += mix(&out, &outbits, bits, bitcount);
            bitcount = enc_count(&bits, c);
            overflow += mix(&out, &outbits, bits, bitcount);
        }

        p = o - order - 1;
        total -= c;
        d = 1;
    }

    out |= cap << outbits;

    if (overflow)
        out |= HASH_OVERFLOW;

    return out;
}


typedef struct hufftree_t hufftree_t;
struct hufftree_t
{
    int freq;
    hufftree_t *zero;
    void *one;
};

void setcode(hufftree_t *x, uint64_t code, int len)
{
    if (x->zero != NULL)
    {
        setcode(x->zero, code, len + 1);
        setcode(x->one, code | (1ull << len), len + 1);
    }
    else
    {
        huffnode_t *n = x->one;
        n->code = code;
        n->codelen = len;
    }
}

static int compare_freq(void const *a, void const *b)
{
    return (*(hufftree_t const * const *)b)->freq - (*(hufftree_t const * const *)a)->freq;
}

void hufftable(huffnode_t node[256], unsigned int hist[256])
{
    hufftree_t *tabptr[256];
    hufftree_t pool[256 + 255];
    int i, j;

    for (i = 0; i < 256; i++)
    {
        pool[i].freq = (int)hist[i] * 16 + 1;
        pool[i].zero = NULL;
        pool[i].one = &node[i];
        tabptr[i] = &pool[i];
    }
    for (j = 256; j > 1; j--)
    {
        qsort(tabptr, j, sizeof(*tabptr), compare_freq);
        pool[i].freq = tabptr[j - 2]->freq + tabptr[j - 1]->freq;
        pool[i].zero = tabptr[j - 2];
        pool[i].one = tabptr[j - 1];
        tabptr[j - 2] = &pool[i++];
    }
    setcode(tabptr[0], 0, 0);
}


void run(int verbose)
{
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        char *o = buffer, *i = buffer;
        while (*i != '\0')
        {
            if (isgraph(*i))
                *o++ = *i;
            i++;
        }
        *o = *i;

        hash_t h = hash(buffer);
        if (verbose)
            printf("%016llx:%s %s\n", (uint64_t)h, h & HASH_OVERFLOW ? "*" : "", buffer);
    }
}

int main(int argc, char *argv[])
{
    while (*++argv != NULL)
    {
        if (strcmp(*argv, "-a") == 0)
            order = alphaorder;
        if (strcmp(*argv, "-h") == 0)
            analyse = 1;
        if (strcmp(*argv, "-c") == 0)
            cap = 1;
    }

    if (analyse)
    {
        run(0);
        rewind(stdin);

        hufftable(first_huff, first_freq);
        hufftable(delta_huff, delta_freq);
        hufftable(count_huff, count_freq);
        huffman = 1;
        analyse = 0;
    }

    run(1);

    return 0;
}
