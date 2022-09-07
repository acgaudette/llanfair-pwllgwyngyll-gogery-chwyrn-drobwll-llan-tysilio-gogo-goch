#define MAX_SYLLABLE 4
#define LEN_OVERFLOW 8
#define LEN_UNDRFLOW 4

#define FINAL_DIFF .75f

#define CONSONANTS \
	ENTRY(B,   1) \
	ENTRY(C,   1) \
	ENTRY(CL,  2) \
	ENTRY(CH,  2) \
	ENTRY(CHD, 3) \
	ENTRY(CN,  2) \
	ENTRY(DD,  2) \
	ENTRY(G,   1) \
	ENTRY(F,   1) \
	ENTRY(FH,  2) \
	ENTRY(GN,  2) \
	ENTRY(H,   1) \
	ENTRY(LL,  2) \
	ENTRY(NG,  2) \
	ENTRY(NN,  2) \
	ENTRY(RH,  2) \
	ENTRY(RR,  2) \
	ENTRY(STR, 3)
#define CONSONANTS_CODA \
	ENTRY(BH,  2) \
	ENTRY(D,   1) \
	ENTRY(DH,  2) \
	ENTRY(GH,  2) \
	ENTRY(L,   1) \
	ENTRY(M,   1) \
	ENTRY(MH,  2) \
	ENTRY(N,   1) \
	ENTRY(R,   1) \
	ENTRY(S,   1) \
	ENTRY(T,   1) \
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
	ENTRY(Y,   1) \
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
// PH
// SR
// WY

#define ENTRY(_, ...) , #_
static const char *consonants[] = { "" CONSONANTS_CODA , "" CONSONANTS };
static const char *vowels[] = {
	"" VOWELS_BROAD VOWELS_BROAD_SLENDER ,
	"" VOWELS_SLENDER VOWELS_SLENDER_BROAD
};
#undef ENTRY

typedef struct {
#define ENTRY(_, ...) , CONS_ ## _
	enum consonant {
		CONS_NONE
		CONSONANTS_CODA , CONS_COUNT_CODA
		CONSONANTS , CONS_LAST
	} onset, coda;
#undef ENTRY
#define ENTRY(_, ...) , VOWEL_ ## _
	enum vowel {
		VOWEL_NONE
		VOWELS_BROAD VOWELS_BROAD_SLENDER , VOWEL_COUNT_BROAD
		VOWELS_SLENDER VOWELS_SLENDER_BROAD , VOWEL_LAST
	} vowel;
#undef ENTRY
} syllable;

#define CONS_COUNT (CONS_LAST - 1)
#define VOWEL_COUNT (VOWEL_LAST - 1)
#define VOWEL_COUNT_SLENDER (VOWEL_COUNT - VOWEL_COUNT_BROAD)

typedef struct {
	u32 len;
	enum {
		  CONSTR_NONE
		, CONSTR_INITIAL
		, CONSTR_FINAL
	} type;
	struct {
		enum vowel_class {
			  PREV_NONE
			, PREV_BROAD
			, PREV_SLENDER
		} class;
		enum consonant cons;
	} prev;
} constraint;

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
static const u32 consonant_len[] = { 0 CONSONANTS_CODA , 0 CONSONANTS };
static const u32 vowel_len[] = {
	0 VOWELS_BROAD VOWELS_BROAD_SLENDER ,
	0 VOWELS_SLENDER VOWELS_SLENDER_BROAD
};
#undef ENTRY

#define RAND_CODA() (randu32(1, CONS_COUNT_CODA))

static enum consonant rand_cons()
{
	enum consonant result = randu32(1, CONS_LAST - 1);
	return result >= CONS_COUNT_CODA ? result + 1 : result;
}

#define RAND_BROAD()   (randu32(1, VOWEL_COUNT_BROAD))
#define RAND_SLENDER() (randu32(VOWEL_COUNT_BROAD + 1, VOWEL_LAST))

static enum vowel rand_vowel()
{
	enum vowel result = randu32(1, VOWEL_LAST - 1);
	return result >= VOWEL_COUNT_BROAD ? result + 1 : result;
}

static int cons_valid(const constraint constr, const enum consonant cons)
{
	if (constr.type != CONSTR_INITIAL && constr.prev.cons) {
		if (constr.prev.cons == cons)
			return 0;
		if (*consonants[constr.prev.cons] == *consonants[cons])
			return 0;
		if (consonant_len[constr.prev.cons] > 1 && consonant_len[cons] > 1)
			return 0;

		switch (cons) {
		case CONS_F:
			switch (*consonants[constr.prev.cons]) {
			case 'D':
			case 'L':
			case 'M':
			case 'N':
			case 'R':
				break;
			default:
				return 0;
			}

			break;
		case CONS_H:
			if (consonant_len[constr.prev.cons] > 1)
				return 0;
			break;
		case CONS_CHD:
		case CONS_NG:
		case CONS_RR:
			return 0;
		default:
			break;
		}

		switch (constr.prev.cons) {
		case CONS_T:
		case CONS_TH:
			switch (*consonants[cons]) {
			case 'R':
			case 'S':
				break;
			default:
				return 0;
			}

			break;
		default:
			break;
		}
	}

	switch (cons) {
	case CONS_CN:
	case CONS_CL:
	case CONS_FH:
	case CONS_GN:
		return constr.type == CONSTR_INITIAL;
	case CONS_CHD:
		return constr.type == CONSTR_FINAL;
	case CONS_CH:
	case CONS_DD:
	case CONS_LL:
	case CONS_MH:
	case CONS_NG:
	case CONS_NN:
	case CONS_RR:
		return constr.type != CONSTR_INITIAL;
	case CONS_M:
		return constr.type != CONSTR_NONE;
	case CONS_H:
	case CONS_S:
	case CONS_STR:
		return constr.type != CONSTR_FINAL;
	default:
		return 1;
	}
}

static syllable gen_syllable(const constraint constr)
{
	syllable result = { 0 };

	const int has_cons = (constr.type == CONSTR_INITIAL) ?
		randf() > .5f : 1;
	if (has_cons) {
		do {
			result.onset = rand_cons();
		} while(!cons_valid(constr, result.onset));
	}

	if (constr.type != CONSTR_FINAL) {
		if (randf() > FINAL_DIFF)
			result.coda = RAND_CODA();
	}

	assert(result.onset != CONS_COUNT_CODA);
	assert(result.coda != CONS_COUNT_CODA);
	assert(result.onset < CONS_LAST);
	assert(result.coda < CONS_LAST);

	switch (result.onset) {
	case CONS_CHD:
		result.vowel = VOWEL_NONE;
		break;
	case CONS_NONE:
		do {
			result.vowel = rand_vowel();
		} while (constr.len == 1 && vowel_len[result.vowel] == 1);
		break;
	default:
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
			switch (constr.prev.class) {
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
	}

	return result;
}

static constraint constraint_prev(
	const syllable prev,
	const int final,
	const u32 len
) {
	const constraint result = {
		.len = len,
		.type = final ? CONSTR_FINAL : CONSTR_NONE,
		.prev = {
			.class = vowel_class_tail(prev.vowel),
			.cons = prev.coda ? prev.coda : CONS_NONE,
		},
	};

	return result;
}

static istr syllables_render(const syllable *in, const u32 n)
{
	char swap[9 * n];
	char *ptr = swap;

	for (u32 i = 0; i < n; ++i) {
		const syllable *s = in + i;

		assert(s->onset | s->vowel | s->coda);
		if (s->onset && s->coda)
			assert(s->vowel);

		if (s->onset) {
			const u32 len = consonant_len[s->onset];
			strncpy(ptr, consonants[s->onset], len);
			ptr += len;
		}

		if (s->vowel) {
			const u32 len = vowel_len[s->vowel];
			strncpy(ptr, vowels[s->vowel], len);
			ptr += len;
		}

		if (s->coda) {
			const u32 len = consonant_len[s->coda];
			strncpy(ptr, consonants[s->coda], len);
			ptr += len;
		}
	}

	*ptr = 0;

	return intern_str(swap);
}

static void syllables_trace(const syllable *in, const u32 n)
{
	for (u32 i = 0; i < n; ++i) {
		const syllable *s = in + i;
		if (s->onset)
			printf("<%s>", consonants[s->onset]);
		if (s->vowel)
			printf("<%s>", vowels[s->vowel]);
		if (s->coda)
			printf("<%s>", consonants[s->coda]);
		printf(" ");
	}
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
			constraint_prev(swap[i - 1], 0, n)
		);
	}

	if (n > 1) {
		// Generate single words; no finals allowed mid word
		swap[n - 1] = gen_syllable(
			constraint_prev(swap[n - 2], 1, n)
		);
	}

	const istr result = syllables_render(swap, n);

	if (strlen(result) > LEN_OVERFLOW)
		return gen_name();
	if (strlen(result) < LEN_UNDRFLOW)
		return gen_name();

#ifdef GEN_TRACE
	syllables_trace(swap, n);
	printf("\n");
#endif
	return result;
}
