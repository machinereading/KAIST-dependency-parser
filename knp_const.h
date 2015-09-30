#ifndef KNP_CONST_H
#define KNP_CONST_H

namespace Parser {

#define BonusRule_MAX	16
#define KoouRule_MAX	124
#define DpndRule_MAX	128
#define DpndRule_G_MAX	1024
#define ContRule_MAX	256
#define DicForRule_MAX	1024
#define NERule_MAX	512
#define CNRule_MAX	512
#define EtcRule_MAX	1024
#define GeneralRule_MAX	1024

#define WORD_LEN_MAX	128
#ifndef IMI_MAX
	#define IMI_MAX	1024	/* defined in "juman.h" */	
#endif
#define DATA_LEN	5120

#define HomoRuleType 1
#define MorphRuleType 2
#define PhraseRuleType 4
#define CFGRuleType 8
#define LatticeMorphRuleType 18
#define LatticePhraseRuleType 20
#define DpndRuleType 32

#define RLOOP_MRM	0
#define RLOOP_RMM	1

#define RLOOP_BREAK_NONE	0
#define RLOOP_BREAK_NORMAL	1
#define RLOOP_BREAK_JUMP	2

#define LtoR		1
#define RtoL		-1

/*====================================================================
			       FEATURE
====================================================================*/

#define RF_MAX	16

// FEATURE structure
typedef struct _FEATURE *FEATUREptr;
typedef struct _FEATURE {
    char	*cp;
    FEATUREptr	next;
    void	_print(std::ostream &os, const char *open_bracket, const char *closed_bracket);
    void	print();
    void	print(std::ostream &os);
    void	print_xml();
    void	print_xml(std::ostream &os);
} FEATURE;

// FEATURE pattern
typedef struct {
    FEATURE 	*fp[RF_MAX];
} FEATURE_PATTERN;


/*====================================================================
                      Pseudo Regular Expressions
====================================================================*/

#define NOT_FLG '^'
#define MAT_FLG '\0'
#define AST_FLG '*'
#define QST_FLG '?'
#define NOT_STR "^"
#define AST_STR "*"
#define QST_STR "?"
#define FW_MATCHING 0
#define BW_MATCHING 1
#define ALL_MATCHING 0
#define PART_MATCHING 1
#define SHORT_MATCHING 0
#define LONG_MATCHING 1

#define RM_HINSHI_MAX 64
#define RM_BUNRUI_MAX 64
#define RM_KATA_MAX 64
#define RM_KEI_MAX  64
#define RM_GOI_MAX  64

// Morphme pattern
typedef struct {
    char type_flag;	/* '?' or '^' or NULL */
    char ast_flag;	/* '*' or NULL */
    char Hinshi_not;
    int Hinshi[RM_HINSHI_MAX];
    char Bunrui_not;
    int Bunrui[RM_BUNRUI_MAX];
    char Kata_not;
    int Katuyou_Kata[RM_KATA_MAX];
    char Kei_not;
    char *Katuyou_Kei[RM_KEI_MAX];
    char Goi_not;
    char *Goi[RM_GOI_MAX];
    FEATURE_PATTERN f_pattern;
} REGEXPMRPH;

// Morpheme sequence pattern
typedef struct {
    REGEXPMRPH 	*mrph;
    char 	mrphsize;
} REGEXPMRPHS;

// Phrase pattern
typedef struct {
    char 	type_flag;	// '?' or '^' or NULL
    char 	ast_flag;	// '*' or NULL
    REGEXPMRPHS	*mrphs;
    FEATURE_PATTERN f_pattern;
} REGEXPPHRASE;

// Phrase sequence pattern
typedef struct {
    REGEXPPHRASE	*bnst;
    char		bnstsize;
} REGEXPPHRASES;


/*====================================================================
                                Rules
====================================================================*/

#define LOOP_BREAK	0
#define LOOP_ALL	1

// Homonym rule
typedef struct {
    REGEXPMRPHS	*pre_pattern;
    REGEXPMRPHS *pattern;
    FEATURE	*f;
} HomoRule;

// CFG rule
typedef struct {
    REGEXPMRPHS *pre_pattern;
    REGEXPMRPHS *post_pattern;
    parser_data_type		pre_pattern_type;
    parser_data_type		post_pattern_type;
    FEATURE	*f;
} CFGRule;

// Morpheme rule
typedef struct {
    REGEXPMRPHS	*pre_pattern;
    REGEXPMRPHS	*self_pattern;
    REGEXPMRPHS	*post_pattern;
    FEATURE	*f;
} MrphRule;

// Phrase rule
typedef struct {
    REGEXPPHRASES	*pre_pattern;
    REGEXPPHRASES	*self_pattern;
    REGEXPPHRASES	*post_pattern;
    FEATURE	*f;
} PhraseRule;

// Dependency rule
typedef struct {
    FEATURE_PATTERN dependant;
    FEATURE_PATTERN governor[DpndRule_G_MAX];
    char	    dpnd_type[DpndRule_G_MAX];
    FEATURE_PATTERN barrier;
    int 	    preference;
    int		    decide;	/* whether it is decided or not(possibility) */
} DpndRule;

// Bonus rule
typedef struct {
    REGEXPMRPHS *pattern;
    int		type;		/* coordination type */
} BonusRule;

// Koou (concord expressions) rule
typedef struct {
    REGEXPMRPHS 	*start_pattern;
    REGEXPMRPHS 	*end_pattern;
    REGEXPMRPHS 	*uke_pattern;
    char		dpnd_type;
} KoouRule;


extern void list2feature_pattern(FEATURE_PATTERN *f, CELL *cell);
extern int feature_pattern_match(FEATURE_PATTERN *fr, FEATURE *fd, void *p1, void *p2);
extern void store_regexpmrphs(REGEXPMRPHS **mspp, CELL *cell);
extern void store_regexpphrases(REGEXPPHRASES **bspp, CELL *cell);
extern void list2feature(CELL *cp, FEATURE **fpp);
extern void assign_feature(FEATURE **fpp1, FEATURE **fpp2, void *ptr, int offset, int length, int temp_assign_flag);

#define FREQ0_ASSINED_SCORE     -13.815511 /* log(0.0000010) */
#define	UNKNOWN_CASE_SCORE	-11.512925 /* log(0.0000100) */
#define NOMINAL_CASEFRAME_SCORE 0
#define	NIL_ASSINED_SCORE	-20
#define VP_MODIFY_SCORE         -20
#define	CASE_MATCH_FAILURE_PROB	-1001
#define	HARSH_PENALTY_SCORE	-500

#define INPUT_IS_MORPHEME 1
#define INPUT_IS_PHRASE 2

#define OUTPUT_IS_TAB 1
#define OUTPUT_IS_XML 2
#define OUTPUT_IS_TREE 4

#define LANGUAGE_IS_JAPANESE 1
#define LANGUAGE_IS_GENERIC 2

#define	DEFAULT_CASE_FRAME_ID "DEFAULT"
}

#endif
