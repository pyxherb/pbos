#include <cltlex.h>
#include <cltparse.hh>
#include "global.h"
#include <cstdarg>
#include <cstdio>

std::unordered_map<std::string, std::shared_ptr<Subset>> globalSubsets;
std::shared_ptr<Subset> currentSubset;

class ArgumentError : public std::runtime_error {
public:
	inline ArgumentError(std::string msg) : runtime_error(msg){};
	virtual inline ~ArgumentError() {}
};

void yy::parser::error(const yy::parser::location_type& loc, const std::string& msg) {
	std::fprintf(stderr, "Error at %d,%d: %s\n", loc.begin.line, loc.begin.column, msg.c_str());
}

static inline char* cmdFetchArg(int argc, char** argv, int& i) {
	if (i >= argc)
		throw ArgumentError("Missing arguments");
	return argv[i++];
}

int main(int argc, char** argv) {
	try {
		std::string srcPath = "", outPath = "a.out";

		try {
			for (int i = 1; i < argc; ++i) {
				std::string arg = cmdFetchArg(argc, argv, i);

				if (arg == "--output" || arg == "-o") {
					outPath = cmdFetchArg(argc, argv, i);
				} else
					srcPath = arg;
			}
		} catch (ArgumentError& e) {
			puts(e.what());
			return EINVAL;
		}

		if (!srcPath.length()) {
			puts("Missing input file");
			return EINVAL;
		}

		if (!(yyin = std::fopen(srcPath.c_str(), "rb"))) {
			printf("Error opening file `%s'", srcPath.c_str());
			return ENOENT;
		}

		auto cleanup = []() {
			currentSubset.reset();
			yylex_destroy();
			yyparser.reset();
		};

		yyparser = std::make_shared<yy::parser>();
		auto parseResult = yyparser->parse();

		fclose(yyin);

		if (parseResult) {
			cleanup();
			return -1;
		}

		for (auto i : globalSubsets) {
			printf("%ssubset %s {\n", i.second->flags & SUBSET_CONST ? "const " : "", i.first.c_str());
			for (auto j : i.second->entries) {
				printf("\t%s[%u] = %s\n", i.first.c_str(), j.first, j.second.c_str());
			}
			printf("}\n");
		}

		cleanup();
	} catch (yy::parser::syntax_error e) {
		std::fprintf(stderr, "%s", e.what());
		return -1;
	} catch (std::bad_alloc) {
		fprintf(stderr, "Out of memory");
		return ENOMEM;
	}

	return 0;
}
