#include "z.h"

#if SPELL
#include <aspell.h>
#include <dlfcn.h>

#define ZDLSYM(A, B, args...) A(*B)(args) = (A(*)(args))dlsym(dl, #B)

void Zspell_word(void)
{
	void *dl = dlopen("libaspell.so", RTLD_LAZY);
	if (!dl) {
		error("You do not have libaspell.so");
		return;
	}

	dlerror(); /* clear errors */
	ZDLSYM(AspellConfig *, new_aspell_config);
	ZDLSYM(void , aspell_config_replace,
	       AspellConfig*, char*, char*);
	ZDLSYM(AspellCanHaveError *, new_aspell_speller,
	       AspellConfig*);
	ZDLSYM(int , aspell_error_number,
	       AspellCanHaveError*);
	ZDLSYM(AspellSpeller *, to_aspell_speller,
	       AspellCanHaveError*);
	ZDLSYM(int , aspell_speller_check,
	       AspellSpeller*, char*, int);
	ZDLSYM(const AspellWordList *, aspell_speller_suggest,
	       AspellSpeller*, char*, int);
	ZDLSYM(AspellStringEnumeration *, aspell_word_list_elements,
	       const AspellWordList*);
	ZDLSYM(const char *, aspell_string_enumeration_next,
	       AspellStringEnumeration*);
	ZDLSYM(void , delete_aspell_string_enumeration,
	       AspellStringEnumeration*);
	ZDLSYM(void , delete_aspell_speller,
	       AspellSpeller*);
	ZDLSYM(void , delete_aspell_config,
	       AspellConfig*);
	if (dlerror()) {
		error("You have an incomplete libaspell.so");
		dlclose(dl);
		return;
	}

	AspellConfig *config = new_aspell_config();
	if (!config) {
		error("Aspell failed");
		dlclose(dl);
		return;
	}

	aspell_config_replace(config, (char *)"lang", (char *)"en_US");

	AspellCanHaveError *possible_err = new_aspell_speller(config);
	if (aspell_error_number(possible_err)) {
		error("Aspell failed");
		dlclose(dl);
		return;
	}
	AspellSpeller *speller = to_aspell_speller(possible_err);

	char word[STRMAX];
	getbword(word, sizeof(word), bisword);
	int size = strlen(word);
	int correct = aspell_speller_check(speller, word, size);

	if (correct)
		putpaw("%s correct", word);
	else {
		int n, i = 0, len = Colmax;

		const AspellWordList *suggestions =
			aspell_speller_suggest(speller, word, size);
		AspellStringEnumeration *elements =
			aspell_word_list_elements(suggestions);
		const char *s;
		while ((s = aspell_string_enumeration_next(elements)) &&
		       len > 0) {
			if (i == 0)
				n = snprintf(PawStr + i, len, "%s", s);
			else
				n = snprintf(PawStr + i, len, " %s", s);
			i += n;
			len -= n;
		}
		delete_aspell_string_enumeration(elements);

		putpaw(PawStr);
	}

	delete_aspell_speller(speller);
	delete_aspell_config(config);
}
#else
void Zspell_word(void) { tbell(); }
#endif
