/* radare - LGPL - Copyright 2009-2010 */
/*   nibble<.ds@gmail.com> */
/*   pancake<nopcode.org> */

#ifndef _INCLUDE_R_ANAL_H_
#define _INCLUDE_R_ANAL_H_

#include <r_types.h>
#include <list.h>
#include <r_reg.h>
#include <r_list.h>
#include <r_util.h>

//TODO: use RList
// deprecate this macro?
#define R_ANAL_MAXREG 16

enum {
	R_ANAL_OP_FAMILY_UNKNOWN = 0,
	R_ANAL_OP_FAMILY_CPU,  /* normal cpu insturction */
	R_ANAL_OP_FAMILY_FPU,  /* fpu (floating point) */
	R_ANAL_OP_FAMILY_MMX,  /* multimedia instruction (packed data) */
	R_ANAL_OP_FAMILY_PRIV, /* priviledged instruction */
	R_ANAL_OP_FAMILY_LAST
};

enum {
	R_ANAL_OP_TYPE_NULL  = 0x0,
	R_ANAL_OP_TYPE_JMP   = 0x1,  /* mandatory jump */
	R_ANAL_OP_TYPE_UJMP  = 0x2,  /* unknown jump (register or so) */
	R_ANAL_OP_TYPE_CJMP  = 0x4,  /* conditional jump */
	R_ANAL_OP_TYPE_CALL  = 0x8,  /* call to subroutine (branch+link) */
	R_ANAL_OP_TYPE_UCALL = 0x10, /* unknown call (register or so) */
	R_ANAL_OP_TYPE_REP   = 0x20, /* repeats next instruction N times */
	R_ANAL_OP_TYPE_RET   = 0x40, /* returns from subrutine */
	R_ANAL_OP_TYPE_ILL   = 0x80,  /* illegal instruction // trap */
	R_ANAL_OP_TYPE_UNK   = 0x100, /* unknown opcode type */
	R_ANAL_OP_TYPE_NOP   = 0x200, /* does nothing */
	R_ANAL_OP_TYPE_MOV   = 0x400, /* register move */
	R_ANAL_OP_TYPE_TRAP  = 0x800, /* it's a trap! */
	R_ANAL_OP_TYPE_SWI   = 0x1000,  /* syscall, software interrupt */
	R_ANAL_OP_TYPE_UPUSH = 0x2000, /* unknown push of data into stack */
	R_ANAL_OP_TYPE_PUSH  = 0x4000,  /* push value into stack */
	R_ANAL_OP_TYPE_POP   = 0x8000,   /* pop value from stack to register */
	R_ANAL_OP_TYPE_CMP   = 0x10000,  /* copmpare something */
	R_ANAL_OP_TYPE_ADD   = 0x20000,
	R_ANAL_OP_TYPE_SUB   = 0x40000,
	R_ANAL_OP_TYPE_MUL   = 0x100000,
	R_ANAL_OP_TYPE_DIV   = 0x200000,
	R_ANAL_OP_TYPE_SHR   = 0x400000,
	R_ANAL_OP_TYPE_SHL   = 0x800000,
	R_ANAL_OP_TYPE_OR    = 0x1000000,
	R_ANAL_OP_TYPE_AND   = 0x2000000,
	R_ANAL_OP_TYPE_XOR   = 0x4000000,
	R_ANAL_OP_TYPE_NOT   = 0x8000000,
	R_ANAL_OP_TYPE_STORE = 0x10000000,  /* store from register to memory */
	R_ANAL_OP_TYPE_LOAD  = 0x20000000,  /* load from memory to register */
};

/* TODO: what to do with signed/unsigned conditionals? */
enum {
	R_ANAL_COND_EQ = 0,
	R_ANAL_COND_NE,
	R_ANAL_COND_GE,
	R_ANAL_COND_GT,
	R_ANAL_COND_LE,
	R_ANAL_COND_LT,
};

enum {
	R_ANAL_VAR_TYPE_NULL   = 0,
	R_ANAL_VAR_TYPE_GLOBAL = 0x01,
	R_ANAL_VAR_TYPE_LOCAL  = 0x02,
	R_ANAL_VAR_TYPE_ARG    = 0x04,
	R_ANAL_VAR_TYPE_ARGREG = 0x08,
	R_ANAL_VAR_TYPE_RET    = 0x10,
};
typedef enum {
	R_ANAL_VAR_DIR_NONE = 0,
	R_ANAL_VAR_DIR_IN   = 0x100,
	R_ANAL_VAR_DIR_OUT  = 0x200
} _RAnalVarDir;

typedef enum {
	R_ANAL_DATA_NULL = 0,
	R_ANAL_DATA_HEX,      /* hex byte pairs */
	R_ANAL_DATA_STR,      /* ascii string */
	R_ANAL_DATA_CODE,     /* plain assembly code */
	R_ANAL_DATA_FUN,      /* plain assembly code */
	R_ANAL_DATA_STRUCT,   /* memory */
	R_ANAL_DATA_LAST
} _RAnalData;

typedef enum {
	R_ANAL_BB_TYPE_NULL = 0,
	R_ANAL_BB_TYPE_HEAD = 0x1,     /* first block */
	R_ANAL_BB_TYPE_BODY = 0x2,     /* conditional jump */
	R_ANAL_BB_TYPE_LAST = 0x4,     /* ret */
	R_ANAL_BB_TYPE_FOOT = 0x8,     /* unknown jump */
	R_ANAL_BB_TYPE_SWITCH = 0x10   /* TODO: switch */
} _RAnalBlockType;

enum {
	R_ANAL_STACK_NULL = 0,
	R_ANAL_STACK_NOP,
	R_ANAL_STACK_INCSTACK,
	R_ANAL_STACK_GET,
	R_ANAL_STACK_SET,
};

typedef enum {
	R_ANAL_REFLINE_TYPE_STYLE = 1,
	R_ANAL_REFLINE_TYPE_WIDE = 2,
} _RAnalReflineType;

enum {
	R_ANAL_RET_ERROR = -1,
	R_ANAL_RET_DUP = -2,
	R_ANAL_RET_NEW = -3,
	R_ANAL_RET_END = -4
};

typedef struct r_anal_t {
	int bits;
	int big_endian;
	int split;
	void *user;
	RList *bbs;
	RList *fcns;
	RList *refs;
	RList *vartypes;
	RReg *reg;
	struct r_anal_ctx_t *ctx;
	struct r_anal_plugin_t *cur;
	struct list_head anals;
} RAnal;

// mul*value+regbase+regidx+delta
typedef struct r_anal_value_t {
	int absolute; // if true, unsigned cast is used
	int memref; // is memory reference? which size? 1, 2 ,4, 8
	ut64 base ; // numeric address
	st64 delta; // numeric delta
	st64 imm; // immediate value
	int mul; // multiplier (reg*4+base)
	ut16 sel; // segment selector
	RRegItem *reg; // register index used (-1 if no reg)
	RRegItem *regdelta; // register index used (-1 if no reg)
} RAnalValue;

typedef struct r_anal_aop_t {
	char *mnemonic; /* mnemonic */
	ut64 addr;      /* address */
	int type;       /* type of opcode */
	int stackop;    /* operation on stack? */
	int cond;       /* condition type */
	int length;     /* length in bytes of opcode */
	int nopcode;    /* number of bytes representing the opcode (not the arguments) */
	int family;     /* family of opcode */
	int eob;        /* end of block (boolean) */
	ut64 jump;      /* true jmp */
	ut64 fail;      /* false jmp */
	ut32 selector;  /* segment selector */
	st64 ref;       /* reference to memory */ /* XXX signed? */
	ut64 value;     /* reference to value */ /* XXX signed? */
	st64 stackptr;  /* stack pointer */
	RAnalValue *src[3];
	RAnalValue *dst;
	int refptr;
} RAnalOp;

#define R_ANAL_COND_SINGLE(x) (!x->arg[1] || x->arg[0]==x->arg[1])

typedef struct r_anal_cond_t {
	int type; // filled by CJMP opcode
	RAnalValue *arg[2]; // filled by CMP opcode
} RAnalCond;

enum {
	R_ANAL_DIFF_TYPE_NULL = 0,
	R_ANAL_DIFF_TYPE_MATCH = 'm',
	R_ANAL_DIFF_TYPE_UNMATCH = 'u'
};

typedef struct r_anal_diff_t {
	int type;
	ut64 addr;
	char *name;
} RAnalDiff;

typedef struct r_anal_bb_t {
	ut64 addr;
	ut64 size;
	ut64 jump;
	ut64 fail;
	int type;
	int ninstr;
	int ncalls;
	int conditional;
	int traced;
	ut8 *fingerprint;
	RAnalDiff *diff;
	RList *aops;
	RAnalCond *cond;
} RAnalBlock;

#if 0
// TODO: add other call convections here
enum {
	R_ANAL_CALL_FAST='f',
	R_ANAL_CALL_SLOW='s',
};
// XXX: must be defined by the function signature!!11
typedef struct r_anal_call_t {
	int type; // fast, stack/slow
	RAnalValue *args[16]; // XXX
} RAnalCall;
#endif

enum {
	R_ANAL_FCN_TYPE_NULL = 0,
	R_ANAL_FCN_TYPE_FCN,
	R_ANAL_FCN_TYPE_LOC
} RAnalFcnType;

typedef struct r_anal_fcn_t {
	char *name;
	ut64 addr;
	ut64 size;
	int type;
	int fastcall; /* non-zero if following fastcall convention */
	int stack;
	int ninstr;
	int nargs;
	ut8 *fingerprint;
	RAnalDiff *diff;
	RList *vars;
	RList *refs;
	RList *xrefs;
} RAnalFcn;

typedef struct r_anal_var_access_t {
	ut64 addr;
	int set;
} RAnalVarAccess;

typedef struct r_anal_var_t {
	char *name;    /* name of the variable */
	ut64 addr; // not used correctly?
	ut64 eaddr; // not used correctly?
	int delta;     /* delta offset inside stack frame */
	int type;      /* global, local... | in, out... */
	int array;     /* array size */
	char *vartype; /* float, int... */
	/* probably dupped or so */
	RList/*RAnalVarAccess*/ *accesses; /* list of accesses for this var */
	RList/*RAnalValue*/ *stores;   /* where this */
} RAnalVar;

typedef struct r_anal_var_type_t {
	char *name;
	char *fmt;
	unsigned int size;
} RAnalVarType;

enum {
	R_ANAL_REF_TYPE_NULL = 0,
	R_ANAL_REF_TYPE_CODE = 'c', // code ref
	R_ANAL_REF_TYPE_CALL = 'C', // code ref (call)
	R_ANAL_REF_TYPE_DATA = 'd'  // mem ref
} RAnalRefType;

typedef struct r_anal_ref_t {
	int type;
	ut64 addr;
	ut64 at;
} RAnalRef;

typedef struct r_anal_refline_t {
	ut64 from;
	ut64 to;
	int index;
	struct list_head list;
} RAnalRefline;

typedef int (*RAnalCallback)(RAnal *a, RAnalOp *aop, ut64 addr, const ut8 *data, int len);

typedef struct r_anal_plugin_t {
	char *name;
	char *desc;
	int (*init)(void *user);
	int (*fini)(void *user);
	RAnalCallback aop;
	struct list_head list;
} RAnalPlugin;

#ifdef R_API
/* anal.c */
R_API RAnal *r_anal_new();
R_API RAnal *r_anal_free(RAnal *r);
R_API void r_anal_set_user_ptr(RAnal *anal, void *user);
R_API int r_anal_add(RAnal *anal, struct r_anal_plugin_t *foo);
R_API int r_anal_list(RAnal *anal);
R_API int r_anal_use(RAnal *anal, const char *name);
R_API int r_anal_set_bits(RAnal *anal, int bits);
R_API int r_anal_set_big_endian(RAnal *anal, int boolean);
R_API char *r_anal_strmask (RAnal *anal, const char *data);

/* bb.c */
R_API RAnalBlock *r_anal_bb_new();
R_API RList *r_anal_bb_list_new();
R_API void r_anal_bb_free(void *bb);
R_API void r_anal_bb_trace(RAnal *anal, ut64 addr);
R_API int r_anal_bb(RAnal *anal, RAnalBlock *bb,
		ut64 addr, ut8 *buf, ut64 len, int head);
R_API int r_anal_bb_split(RAnal *anal, RAnalBlock *bb,
		RList *bbs, ut64 addr);
R_API int r_anal_bb_overlap(RAnal *anal, RAnalBlock *bb, RList *bbs);
R_API int r_anal_bb_add(RAnal *anal, ut64 addr,
		ut64 size, ut64 jump, ut64 fail, int type, RAnalDiff *diff);
R_API int r_anal_bb_del(RAnal *anal, ut64 addr);

/* aop.c */
R_API RAnalOp *r_anal_aop_new();
R_API char *r_anal_aop_to_string(RAnal *anal, RAnalOp *op);
R_API void r_anal_aop_free(void *aop);
R_API RList *r_anal_aop_list_new();
R_API int r_anal_aop(RAnal *anal, RAnalOp *aop, ut64 addr,
		const ut8 *data, int len);

/* fcn.c */
R_API RAnalFcn *r_anal_fcn_new();
R_API RAnalFcn *r_anal_fcn_find(RAnal *anal, ut64 addr, int type);
R_API RList *r_anal_fcn_list_new();
R_API void r_anal_fcn_free(void *fcn);
R_API int r_anal_fcn(RAnal *anal, RAnalFcn *fcn, ut64 addr,
		ut8 *buf, ut64 len, int reftype);
R_API int r_anal_fcn_add(RAnal *anal, ut64 addr, ut64 size,
		const char *name, int type, RAnalDiff *diff);
R_API int r_anal_fcn_del(RAnal *anal, ut64 addr);
R_API RList *r_anal_fcn_bb_list(RAnal *anal, RAnalFcn *fcn);
R_API RAnalVar *r_anal_fcn_get_var(RAnalFcn *fs, int num, int dir);
R_API char *r_anal_fcn_to_string(RAnal *a, RAnalFcn* fs);
R_API int r_anal_fcn_from_string(RAnal *a, RAnalFcn *f, const char *_str);

/* ref.c */
R_API RAnalRef *r_anal_ref_new();
R_API RList *r_anal_ref_list_new();
R_API void r_anal_ref_free(void *ref);
R_API int r_anal_ref_add(RAnal *anal, ut64 addr, ut64 at, int type);
R_API int r_anal_ref_del(RAnal *anal, ut64 at);
R_API RList *r_anal_xref_get(RAnal *anal, ut64 addr);

/* var.c */
R_API RAnalVar *r_anal_var_new();
R_API RAnalVarType *r_anal_var_type_new();
R_API RAnalVarAccess *r_anal_var_access_new();
R_API RList *r_anal_var_list_new();
R_API RList *r_anal_var_type_list_new();
R_API RList *r_anal_var_access_list_new();
R_API void r_anal_var_free(void *var);
R_API void r_anal_var_type_free(void *vartype);
R_API void r_anal_var_access_free(void *access);
R_API int r_anal_var_type_add(RAnal *anal, const char *name, int size, const char *fmt);
R_API int r_anal_var_type_del(RAnal *anal, const char *name);
R_API RAnalVarType *r_anal_var_type_get(RAnal *anal, const char *name);
R_API int r_anal_var_add(RAnal *anal, RAnalFcn *fcn, ut64 from, int delta, int type,
		const char *vartype, const char *name, int set);
R_API int r_anal_var_del(RAnal *anal, RAnalFcn *fcn, int delta, int type);
R_API RAnalVar *r_anal_var_get(RAnal *anal, RAnalFcn *fcn, int delta, int type);
R_API const char *r_anal_var_type_to_str (RAnal *anal, int type);
R_API int r_anal_var_access_add(RAnal *anal, RAnalVar *var, ut64 from, int set);
R_API int r_anal_var_access_del(RAnal *anal, RAnalVar *var, ut64 from);
R_API RAnalVarAccess *r_anal_var_access_get(RAnal *anal, RAnalVar *var, ut64 from);

/* diff.c */
R_API RAnalDiff *r_anal_diff_new();
R_API void* r_anal_diff_free(RAnalDiff *diff);

R_API RAnalValue *r_anal_value_new();
R_API RAnalValue *r_anal_value_new_from_string(const char *str);
R_API st64 r_anal_value_eval(RAnalValue *value);
R_API char *r_anal_value_to_string (RAnalValue *value);
R_API ut64 r_anal_value_to_ut64(RAnal *anal, RAnalValue *val);
R_API void r_anal_value_free(RAnalValue *value);

R_API RAnalCond *r_anal_cond_new();
R_API RAnalCond *r_anal_cond_new_from_aop(RAnalOp *op);
#define r_anal_cond_free(x) free(x);
R_API char *r_anal_cond_to_string(RAnalCond *cond);
R_API int r_anal_cond_eval (RAnal *anal, RAnalCond *cond);
R_API RAnalCond *r_anal_cond_new_from_string(const char *str);

/* reflines.c */
R_API struct r_anal_refline_t *r_anal_reflines_get(RAnal *anal, 
	ut64 addr, ut8 *buf, ut64 len, int nlines, int linesout, int linescall);
R_API char* r_anal_reflines_str(struct r_anal_t *anal, struct r_anal_refline_t *list,
	ut64 addr, int opts);
R_API int r_anal_reflines_middle(RAnal *anal, RAnalRefline *list, ut64 addr, int len);

/* TO MOVE into r_core */
R_API int r_anal_var_list_show(RAnal *anal, RAnalFcn *fcn, ut64 addr);
R_API int r_anal_var_list(RAnal *anal, RAnalFcn *fcn, ut64 addr, int delta);

/* plugin pointers */
extern RAnalPlugin r_anal_plugin_csr;
extern RAnalPlugin r_anal_plugin_arm;
extern RAnalPlugin r_anal_plugin_x86;
extern RAnalPlugin r_anal_plugin_x86_simple;
extern RAnalPlugin r_anal_plugin_ppc;
extern RAnalPlugin r_anal_plugin_java;
extern RAnalPlugin r_anal_plugin_mips;

#endif
#endif
