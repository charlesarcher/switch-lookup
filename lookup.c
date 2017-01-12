#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define TRACING_SSC_MARK( MARK_ID )                     \
        __asm__ __volatile__ (                          \
                "\n\t  movl $"#MARK_ID", %%ebx"         \
                "\n\t  .byte 0x64, 0x67, 0x90"          \
                : : : "%ebx","memory" );

#define MAPPING_TYPE_DIRECT              0
#define MAPPING_TYPE_DIRECT_INTRA        1
#define MAPPING_TYPE_OFFSET              2
#define MAPPING_TYPE_OFFSET_INTRA        3
#define MAPPING_TYPE_STRIDE              4
#define MAPPING_TYPE_STRIDE_INTRA        5
#define MAPPING_TYPE_STRIDE_BLOCK        6
#define MAPPING_TYPE_STRIDE_BLOCK_INTRA  7
#define MAPPING_TYPE_LUT                 8
#define MAPPING_TYPE_LUT_INTRA           9
#define MAPPING_TYPE_MLUT                10

#define CALC_STRIDE_SIMPLE(rank,stride,offset) \
        ((rank) * (stride) + (offset))

#define CALC_STRIDE(rank,stride,blocksize,offset)   \
        ((rank)/(blocksize)*(stride-blocksize)+(rank)+(offset))

typedef struct {
        uint64_t entry;
} addr_entry_t;


typedef struct {
        uint32_t     size;
        addr_entry_t table[0];
} addr_table_t;

typedef struct {
        uint32_t av_id;
        uint32_t lpid;
}gpid_t;

typedef struct {
        int kind;
        uint32_t av_id;
        uint32_t size;
        union {
                uint32_t offset;
                struct {
                        uint32_t offset;
                        uint32_t blksize;
                        uint32_t stride;
                }stride;
        }regular;
        union {
                struct {
                        uint32_t *table;
                        uint32_t *lpid;
                }lut;
                struct {
                        gpid_t *table;
                        gpid_t *gpid;
                }mlut;
        }irregular;
} communicator_t;

communicator_t comm_direct = {
        .kind = MAPPING_TYPE_DIRECT,
        .size = 1024
};

communicator_t comm_direct_intra = {
        .kind = MAPPING_TYPE_DIRECT_INTRA,
        .size = 1024
};

communicator_t comm_offset = {
        .kind  = MAPPING_TYPE_OFFSET,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .offset = 512
        },
};

communicator_t comm_offset_intra = {
        .kind  = MAPPING_TYPE_OFFSET_INTRA,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .offset = 512
        },
};

communicator_t comm_stride = {
        .kind  = MAPPING_TYPE_STRIDE,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .stride = {
                        .offset  = 512,
                        .blksize = 1,
                        .stride  = 20
                }
        },
};

communicator_t comm_stride_intra = {
        .kind  = MAPPING_TYPE_STRIDE_INTRA,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .stride = {
                        .offset  = 512,
                        .blksize = 1,
                        .stride  = 20
                }
        },
};

communicator_t comm_stride_block = {
        .kind  = MAPPING_TYPE_STRIDE_BLOCK,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .stride = {
                        .offset  = 512,
                        .blksize = 10,
                        .stride  = 20
                }
        },
};

communicator_t comm_stride_block_intra = {
        .kind  = MAPPING_TYPE_STRIDE_BLOCK_INTRA,
        .av_id = 0,
        .size  = 1024,
        .regular  = {
                .stride = {
                        .offset  = 512,
                        .blksize = 10,
                        .stride  = 20
                }
        },
};

communicator_t comm_lut = {
        .kind  = MAPPING_TYPE_LUT,
        .av_id = 0,
        .size  = 1024,
        .irregular  = {
                .lut = {
                        .table = NULL,
                        .lpid  = NULL
                }
        },
};

communicator_t comm_lut_intra = {
        .kind  = MAPPING_TYPE_LUT_INTRA,
        .av_id = 0,
        .size  = 1024,
        .irregular  = {
                .lut = {
                        .table = NULL,
                        .lpid  = NULL
                }
        },
};

communicator_t comm_mlut = {
        .kind  = MAPPING_TYPE_MLUT,
        .av_id = 0,
        .size  = 1024,
        .irregular  = {
                .mlut = {
                        .table = NULL,
                        .gpid  = NULL
                }
        },
};

addr_table_t  *old_addr_table=NULL;
addr_table_t **new_addr_table=NULL;
addr_table_t  *new_addr_table0=NULL;


static inline addr_entry_t __attribute__ ((always_inline)) old_rank_to_addr(int rank)
{
        return old_addr_table->table[rank];
}

static inline addr_entry_t __attribute__ ((always_inline)) old_rank_to_addr_comm(communicator_t *comm,
                                                                                 int             rank)
{
        return old_addr_table->table[comm->irregular.lut.lpid[rank]];
}

static inline addr_entry_t __attribute__ ((always_inline)) new_rank_to_addr(int rank)
{
        return new_addr_table[0]->table[rank];
}

static inline addr_entry_t __attribute__ ((always_inline)) new_rank_to_addr0(int rank)
{
        return new_addr_table0->table[rank];
}

static inline addr_entry_t __attribute__ ((always_inline)) case_rank_to_addr(int rank,
                                                                             int maptype)
{
        addr_entry_t e;
        switch (maptype)
        {
        case MAPPING_TYPE_DIRECT:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_OFFSET:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_STRIDE:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_STRIDE_BLOCK:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_LUT:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_MLUT:
                return new_addr_table[0]->table[rank];
        }
        e.entry = 0ULL;
        return e;
}

static inline addr_entry_t __attribute__ ((always_inline)) case_rank_to_addr_calc(communicator_t *comm,
                                                                                  int             rank)
{
        addr_entry_t e;
        switch (comm->kind)
        {
        case MAPPING_TYPE_DIRECT:
                return new_addr_table[comm->av_id]->table[rank];
        case MAPPING_TYPE_DIRECT_INTRA:
                return new_addr_table0->table[rank];
        case MAPPING_TYPE_OFFSET:
                return new_addr_table[comm->av_id]->table[rank+comm->regular.offset];
        case MAPPING_TYPE_OFFSET_INTRA:
                return new_addr_table0->table[rank+comm->regular.offset];
        case MAPPING_TYPE_STRIDE:
        {
                int idx = CALC_STRIDE_SIMPLE(rank,comm->regular.stride.stride,comm->regular.stride.offset);
                return new_addr_table[comm->av_id]->table[idx];
        }
        case MAPPING_TYPE_STRIDE_INTRA:
        {
                int idx = CALC_STRIDE_SIMPLE(rank,comm->regular.stride.stride,comm->regular.stride.offset);
                return new_addr_table0->table[idx];
        }
        case MAPPING_TYPE_STRIDE_BLOCK:
        {
                int idx = CALC_STRIDE(rank,comm->regular.stride.stride,
                                           comm->regular.stride.blksize,
                                           comm->regular.stride.offset);
                return new_addr_table[comm->av_id]->table[idx];
        }
        case MAPPING_TYPE_STRIDE_BLOCK_INTRA:
        {
                int idx = CALC_STRIDE(rank,comm->regular.stride.stride,
                                           comm->regular.stride.blksize,
                                           comm->regular.stride.offset);
                return new_addr_table0->table[idx];
        }
        case MAPPING_TYPE_LUT:
                return new_addr_table[comm->av_id]->table[comm->irregular.lut.lpid[rank]];
        case MAPPING_TYPE_LUT_INTRA:
                return new_addr_table0->table[comm->irregular.lut.lpid[rank]];
        case MAPPING_TYPE_MLUT:
                return new_addr_table[comm->irregular.mlut.gpid[rank].av_id]->table[comm->irregular.mlut.gpid[rank].lpid];
        }
        e.entry = 0ULL;
        return e;
}

#define RANK_TO_ADDR(in_entry,func,rank, mark)          \
        do {                                            \
                TRACING_SSC_MARK(mark);                 \
                in_entry = func(rank);                    \
                TRACING_SSC_MARK(mark+0x10);              \
        }                                               \
        while(0)

#define RANK_TO_ADDR_TYPE(in_entry,func,type,rank, mark)    \
        do {                                                \
                TRACING_SSC_MARK(mark);                     \
                in_entry = func(rank,type);                 \
                TRACING_SSC_MARK(mark+0x10);                \
        }                                                   \
        while(0)

#define RANK_TO_ADDR_DYNAMIC_TYPE(in_entry,func,type,rank,mark) \
        do {                                                    \
                env = getenv("MAPPING_TYPE");                   \
                if(env) mapping_type = atoi(env);               \
                else mapping_type = type;                       \
                TRACING_SSC_MARK(mark);                         \
                in_entry = func(rank,mapping_type);             \
                TRACING_SSC_MARK(mark+0x10);                    \
        }                                                       \
        while(0)

#define RANK_TO_ADDR_FULL(in_entry,func,in_comm,rank,mark)      \
        do {                                                    \
                env = getenv("COMM_TYPE");                      \
                if(env) comm = (communicator_t*)env;            \
                else comm = in_comm;                            \
                TRACING_SSC_MARK(mark);                         \
                in_entry = func(comm, rank);                    \
                TRACING_SSC_MARK(mark+0x10);                    \
        }                                                       \
        while(0)




int main (int argc, char * argv[])
{
        addr_entry_t old_entry, new_entry, new_entry0, case_entry[24];
        int          mapping_type;
        communicator_t *comm;
        int i;
        char *env;
        const int size    = 1024;
        old_addr_table    = (addr_table_t *)  malloc(size * sizeof(addr_entry_t) + sizeof(addr_table_t));
        new_addr_table    = (addr_table_t **) malloc(1 * sizeof(addr_table_t*));
        new_addr_table[0] = (addr_table_t *)  malloc(size * sizeof(addr_entry_t) + sizeof(addr_table_t));
        new_addr_table0   = new_addr_table[0];

        comm_lut.irregular.lut.table   = malloc(size * sizeof(uint32_t));
        comm_lut.irregular.lut.lpid    = comm_lut.irregular.lut.table;
        for(i=0; i< size; i++)comm_lut.irregular.lut.table[i] = i;

        comm_lut_intra.irregular.lut.table   = malloc(size * sizeof(uint32_t));
        comm_lut_intra.irregular.lut.lpid    = comm_lut_intra.irregular.lut.table;
        for(i=0; i< size; i++)comm_lut_intra.irregular.lut.table[i] = i;

        comm_mlut.irregular.mlut.table = malloc(size * sizeof(gpid_t));
        comm_mlut.irregular.mlut.gpid  = comm_mlut.irregular.mlut.table;
        for(i=0; i< size; i++) {
                comm_mlut.irregular.mlut.table[i].lpid  = i;
                comm_mlut.irregular.mlut.table[i].av_id = 0;
        }
        RANK_TO_ADDR(old_entry,old_rank_to_addr,8,0x200);
        RANK_TO_ADDR(new_entry,new_rank_to_addr,8,0x400);
        RANK_TO_ADDR(new_entry0,new_rank_to_addr0,8,0x600);

        RANK_TO_ADDR_TYPE(case_entry[0],case_rank_to_addr,MAPPING_TYPE_DIRECT,8,0x1000);
        RANK_TO_ADDR_TYPE(case_entry[1],case_rank_to_addr,MAPPING_TYPE_OFFSET,8,0x1200);
        RANK_TO_ADDR_TYPE(case_entry[2],case_rank_to_addr,MAPPING_TYPE_STRIDE,8,0x1400);
        RANK_TO_ADDR_TYPE(case_entry[3],case_rank_to_addr,MAPPING_TYPE_STRIDE_BLOCK,8,0x1600);
        RANK_TO_ADDR_TYPE(case_entry[4],case_rank_to_addr,MAPPING_TYPE_LUT,8,0x1800);
        RANK_TO_ADDR_TYPE(case_entry[5],case_rank_to_addr,MAPPING_TYPE_MLUT,8,0x2000);

        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[6],case_rank_to_addr,MAPPING_TYPE_DIRECT,8,0x2200);
        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[7],case_rank_to_addr,MAPPING_TYPE_OFFSET,8,0x2400);
        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[8],case_rank_to_addr,MAPPING_TYPE_STRIDE,8,0x2600);
        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[9],case_rank_to_addr,MAPPING_TYPE_STRIDE_BLOCK,8,0x2800);
        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[10],case_rank_to_addr,MAPPING_TYPE_LUT,8,0x3000);
        RANK_TO_ADDR_DYNAMIC_TYPE(case_entry[11],case_rank_to_addr,MAPPING_TYPE_MLUT,8,0x3200);

        RANK_TO_ADDR_FULL(case_entry[12],case_rank_to_addr_calc,&comm_direct,8,0x4000);
        RANK_TO_ADDR_FULL(case_entry[13],case_rank_to_addr_calc,&comm_direct_intra,8,0x4200);
        RANK_TO_ADDR_FULL(case_entry[14],case_rank_to_addr_calc,&comm_offset,8,0x4400);
        RANK_TO_ADDR_FULL(case_entry[15],case_rank_to_addr_calc,&comm_offset_intra,8,0x4600);
        RANK_TO_ADDR_FULL(case_entry[16],case_rank_to_addr_calc,&comm_stride,8,0x4800);
        RANK_TO_ADDR_FULL(case_entry[17],case_rank_to_addr_calc,&comm_stride_intra,8,0x5000);
        RANK_TO_ADDR_FULL(case_entry[18],case_rank_to_addr_calc,&comm_stride_block,8,0x5200);
        RANK_TO_ADDR_FULL(case_entry[19],case_rank_to_addr_calc,&comm_stride_block_intra,8,0x5400);
        RANK_TO_ADDR_FULL(case_entry[20],case_rank_to_addr_calc,&comm_lut,8,0x5600);
        RANK_TO_ADDR_FULL(case_entry[21],case_rank_to_addr_calc,&comm_lut_intra,8,0x5800);
        RANK_TO_ADDR_FULL(case_entry[22],case_rank_to_addr_calc,&comm_mlut,8,0x6000);

        RANK_TO_ADDR_FULL(case_entry[23],old_rank_to_addr_comm,&comm_lut,8,0x6200);


        fprintf(stderr, "old_entry=%" PRIx64 ", new_entry=%" PRIx64 " new_entry0=%" PRIx64 "\n",
                old_entry.entry, new_entry.entry, new_entry0.entry);

        fprintf(stderr, "ce0=%" PRIx64 ", ce1=%" PRIx64 ", ce2=%" PRIx64 ", ce3=%" PRIx64 " ce4=%" PRIx64 " ce5=%" PRIx64 "\n",
                case_entry[0].entry, case_entry[1].entry, case_entry[2].entry, case_entry[3].entry,
                case_entry[4].entry, case_entry[5].entry);

        fprintf(stderr, "ce6=%" PRIx64 ", ce7=%" PRIx64 ", ce8=%" PRIx64 ", ce9=%" PRIx64 " ce10=%" PRIx64 " ce11=%" PRIx64 "\n",
                case_entry[6].entry, case_entry[7].entry, case_entry[8].entry, case_entry[9].entry,
                case_entry[10].entry, case_entry[11].entry);

        fprintf(stderr, "ce12=%" PRIx64 ", ce13=%" PRIx64 ", ce14=%" PRIx64 ", ce15=%" PRIx64 " ce16=%" PRIx64 " ce17=%" PRIx64 "\n",
                case_entry[12].entry, case_entry[13].entry, case_entry[14].entry, case_entry[15].entry,
                case_entry[16].entry, case_entry[17].entry);

        fprintf(stderr, "ce18=%" PRIx64 ", ce19=%" PRIx64 ", ce20=%" PRIx64 ", ce21=%" PRIx64 " ce22=%" PRIx64 "ce22=%" PRIx64 "\n",
                case_entry[18].entry, case_entry[19].entry, case_entry[20].entry, case_entry[21].entry,
                case_entry[22].entry, case_entry[23].entry);

        free(new_addr_table[0]);
        free(new_addr_table);
        free(old_addr_table);
        free(comm_lut.irregular.lut.table);
        free(comm_lut_intra.irregular.lut.table);
        free(comm_mlut.irregular.mlut.table);
        return 0;
}
