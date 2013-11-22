#include "z.h"

#if SPELL
#include <aspell.h>

void Zspell_word(void)
{
	char word[STRMAX];
	getbword(word, sizeof(word), bisword);

	AspellConfig *config = new_aspell_config();
	if (!config) {
		putpaw("Aspell failed");
		return;
	}

	aspell_config_replace(config, "lang", "en_US");

	AspellCanHaveError *possible_err = new_aspell_speller(config);
	if (aspell_error_number(possible_err)) {
		putpaw("Aspell failed");
		return;
	}
	AspellSpeller *speller = to_aspell_speller(possible_err);

	int size = strlen(word);
	int correct = aspell_speller_check(speller, word, size);

	if (correct)
		putpaw("%s correct", word);
	else {
		int n, i = 0, len = Colmax;

		const AspellWordList *suggestions = aspell_speller_suggest(speller,
									   word, size);
		AspellStringEnumeration * elements = aspell_word_list_elements(suggestions);
		const char *suggest;
		while ((suggest = aspell_string_enumeration_next(elements)) && len > 0) {
			if (i == 0)
				n = snprintf(PawStr + i, len, "%s", suggest);
			else
				n = snprintf(PawStr + i, len, " %s", suggest);
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
