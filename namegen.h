#define MAX_SYLLABLE 4
#define LEN_OVERFLOW 8
#define LEN_UNDRFLOW 3

#define CONSONANTS \
	ENTRY(B,  1) \
	ENTRY(BH, 2) \
		ENTRY(RBH, 3) \
		ENTRY(LBH, 3) \
	ENTRY(C,  1) \
		ENTRY(RC, 2) \
		ENTRY(LC, 2) \
		ENTRY(NC, 2) \
	ENTRY(CL,  2) \
	ENTRY(CH,  2) \
	ENTRY(CHD, 3) \
	ENTRY(CN,  2) \
	ENTRY(D,   1) \
		ENTRY(RD, 2) \
	ENTRY(DH,  2) \
		ENTRY(RDH, 3) \
	ENTRY(F,   1) \
	ENTRY(FH,  2) \
	ENTRY(G,   1) \
	ENTRY(GH,  2) \
		ENTRY(LGH, 3) \
	ENTRY(GN,  2) \
	ENTRY(L,   1) \
		ENTRY(DHL, 3) \
	ENTRY(LL,  2) \
	ENTRY(M,   1) \
	ENTRY(MH,  2) \
	ENTRY(N,   1) \
		ENTRY(GHN, 3) \
	ENTRY(NG,  2) \
	ENTRY(NN,  2) \
	ENTRY(R,   1) \
		ENTRY(DHR, 3) \
		ENTRY(GHR, 3) \
	ENTRY(RR,  2) \
	ENTRY(S,   1) \
		ENTRY(LLS, 3) \
	ENTRY(SR,  2) \
	ENTRY(STR, 3) \
	ENTRY(T,   1) \
		ENTRY(RT, 2) \
		ENTRY(LT, 2) \
		ENTRY(NT, 2) \
		ENTRY(ST, 2) \
	ENTRY(TH,  2)
#define VOWELS_BROAD \
	ENTRY(A,  1) \
	ENTRY(O,  1) \
	ENTRY(AO, 2) \
	ENTRY(U,  1) \
	ENTRY(UA, 2)
#define VOWELS_BROAD_SLENDER \
	ENTRY(AI,  2) \
	ENTRY(AOI, 3) \
	ENTRY(OI,  2) \
	ENTRY(UAI, 3) \
	ENTRY(UI,  2)
#define VOWELS_SLENDER \
	ENTRY(E,   1) \
	ENTRY(I,   1) \
	ENTRY(EI,  2) \
	ENTRY(EOI, 3)
#define VOWELS_SLENDER_BROAD \
	ENTRY(EA, 2) \
	ENTRY(EO, 2) \
	ENTRY(EU, 2) \
	ENTRY(IA, 2) \
	ENTRY(IO, 2) \
	ENTRY(IU, 2)

// P
// H
// PH

#define ENTRY(_, ...) , #_
static const char *consonants[] = { "" CONSONANTS };
static const char *vowels[] = {
	"" VOWELS_BROAD VOWELS_BROAD_SLENDER ,
	"" VOWELS_SLENDER VOWELS_SLENDER_BROAD
};
#undef ENTRY

typedef struct {
	u32 len;
	enum {
		  CONSTR_NONE
		, CONSTR_INITIAL
		, CONSTR_FINAL
	} type;
	enum vowel_class {
		  PREV_NONE
		, PREV_BROAD
		, PREV_SLENDER
	} prev;
} constraint;

typedef struct {
#define ENTRY(_, ...) , CONS_ ## _
	enum consonant { CONS_NONE CONSONANTS , CONS_COUNT } consonant;
#undef ENTRY
#define ENTRY(_, ...) , VOWEL_ ## _
	enum vowel {
		VOWEL_NONE
		VOWELS_BROAD VOWELS_BROAD_SLENDER , VOWEL_COUNT_BROAD
		VOWELS_SLENDER VOWELS_SLENDER_BROAD , VOWEL_LAST
	} vowel;
#undef ENTRY
} syllable;

#define VOWEL_COUNT (VOWEL_LAST - 1)
#define VOWEL_COUNT_SLENDER (VOWEL_COUNT - VOWEL_COUNT_BROAD)

static const enum vowel_class vowel_class_tail(const enum vowel vowel)
{
	switch (vowel) {
#define ENTRY(_, ...) case VOWEL_ ## _ :
	VOWELS_SLENDER_BROAD
	VOWELS_BROAD
		return PREV_BROAD ;
	VOWELS_BROAD_SLENDER
	VOWELS_SLENDER
		return PREV_SLENDER ;
	default:
		panic();
	}
#undef ENTRY
}

#define ENTRY(_, N) , N
static const u32 consonant_len[] = { 0 CONSONANTS };
static const u32 vowel_len[] = {
	0 VOWELS_BROAD VOWELS_BROAD_SLENDER ,
	0 VOWELS_SLENDER VOWELS_SLENDER_BROAD
};
#undef ENTRY

#define RAND_CONS() (randu32(1, CONS_COUNT))
#define RAND_BROAD()   (randu32(1, VOWEL_COUNT_BROAD))
#define RAND_SLENDER() (randu32(VOWEL_COUNT_BROAD + 1, VOWEL_LAST))

static enum vowel rand_vowel()
{
	enum vowel result = randu32(1, VOWEL_LAST - 1);
	return result >= VOWEL_COUNT_BROAD ? result + 1 : result;
}

static int cons_valid(const constraint constr, const enum consonant cons)
{
	switch (cons) {
	case CONS_CN:
	case CONS_CL:
	case CONS_FH:
	case CONS_GN:
	case CONS_SR:
	case CONS_GHN:
	case CONS_GHR:
		return constr.type == CONSTR_INITIAL;
	case CONS_DHL:
	case CONS_DHR:
	case CONS_LLS:
		return constr.type == CONSTR_NONE;
	case CONS_CHD:
		return constr.type == CONSTR_FINAL;
	case CONS_RBH:
	case CONS_LBH:
	case CONS_RC:
	case CONS_LC:
	case CONS_NC:
	case CONS_LGH:
	case CONS_CH:
	case CONS_RD:
	case CONS_RDH:
	case CONS_LL:
	case CONS_MH:
	case CONS_NG:
	case CONS_NN:
	case CONS_RR:
	case CONS_RT:
	case CONS_LT:
	case CONS_NT:
	case CONS_ST:
		return constr.type != CONSTR_INITIAL;
	case CONS_M:
		return constr.type != CONSTR_NONE;
	case CONS_S:
	case CONS_STR:
		return constr.type != CONSTR_FINAL;
	default:
		return 1;
	}
}

static syllable gen_syllable(const constraint constr)
{
	syllable result;

	const int has_cons = (constr.type == CONSTR_INITIAL) ?
		randf() > .5f : 1;
	if (has_cons) {
		do {
			result.consonant = randu32(1, CONS_COUNT);
		} while(!cons_valid(constr, result.consonant));
	}

	else result.consonant = CONS_NONE;

	switch (result.consonant) {
	case CONS_BH:
		case CONS_RBH:
		case CONS_LBH:
	case CONS_B:
	case CONS_C:
		case CONS_RC:
		case CONS_LC:
		case CONS_NC:
	case CONS_CH:
	case CONS_CN:
	case CONS_CL:
	case CONS_D:
		case CONS_RD:
	case CONS_DH:
		case CONS_RDH:
	case CONS_F:
	case CONS_FH:
	case CONS_G:
	case CONS_GH:
		case CONS_LGH:
	case CONS_GN:
	case CONS_L:
		case CONS_DHL:
	case CONS_LL:
	case CONS_M:
	case CONS_MH:
	case CONS_N:
		case CONS_GHN:
	case CONS_NG:
	case CONS_NN:
	case CONS_R:
		case CONS_DHR:
		case CONS_GHR:
	case CONS_RR:
	case CONS_S:
		case CONS_LLS:
	case CONS_SR:
	case CONS_STR:
	case CONS_T:
		case CONS_RT:
		case CONS_LT:
		case CONS_NT:
		case CONS_ST:
	case CONS_TH:
		switch (constr.type) {
		case CONSTR_INITIAL:
			result.vowel = rand_vowel();
			break;
		case CONSTR_FINAL:
			if (randf() > .5f) {
				result.vowel = VOWEL_NONE;
				break;
			}
		case CONSTR_NONE:
			switch (constr.prev) {
			case PREV_BROAD:
				result.vowel = RAND_BROAD();
				break;
			case PREV_SLENDER:
				result.vowel = RAND_SLENDER();
				break;
			default:
				panic();
			}
			break;
		}
		break;
	case CONS_CHD:
		result.vowel = VOWEL_NONE;
		break;
	case CONS_NONE:
		do {
			result.vowel = rand_vowel();
		} while (constr.len == 1 && vowel_len[result.vowel] == 1);
		break;
	default:
		panic();
	}

	return result;
}

static istr syllables_render(const syllable *in, const u32 n)
{
	char swap[4 * n];
	char *ptr = swap;

	for (u32 i = 0; i < n; ++i) {
		const syllable *s = in + i;
		assert(s->consonant | s->vowel);

		if (s->consonant) {
			const u32 len = consonant_len[s->consonant];
			strncpy(ptr, consonants[s->consonant], len);
			ptr += len;
		}

		if (s->vowel) {
			const u32 len = vowel_len[s->vowel];
			strncpy(ptr, vowels[s->vowel], len);
			ptr += len;
		}
	}

	*ptr = 0;

	return intern_str(swap);
}

istr gen_name()
{
	const u32 n = randu32(1, (MAX_SYLLABLE + 1));
	syllable swap[n];

	swap[0] = gen_syllable(
		(constraint) {
			.len = n,
			.type = CONSTR_INITIAL
		}
	);

	for (u32 i = 1; i < n - 1; ++i) {
		swap[i] = gen_syllable(
			(constraint) {
				.len = n,
				.type = CONSTR_NONE,
				.prev = vowel_class_tail(swap[i - 1].vowel),
			}
		);
	}

	if (n > 1) {
		// Generate single words; no finals allowed mid word
		swap[n - 1] = gen_syllable(
			(constraint) {
				.len = n,
				.type = CONSTR_FINAL,
				.prev = vowel_class_tail(swap[n - 2].vowel),
			}
		);
	}

	const istr result = syllables_render(swap, n);

	if (strlen(result) > LEN_OVERFLOW)
		return gen_name();
	if (strlen(result) < LEN_UNDRFLOW)
		return gen_name();

	return result;
}
