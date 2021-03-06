#include "JsParse.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            cerr<<stderr<<__FILE__<<":"<<__LINE__<<":"<<"expect: "<<expect<<" actual: "<<actual<<endl;\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && actual == expect, expect, actual)
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual)
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual)
#endif

static void test_parse_null() {
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "null";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr,str));
	EXPECT_EQ_INT(JVT_NULL, Jsptr->GetType());
}

static void test_parse_true() {
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "true";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));
	EXPECT_EQ_INT(JVT_TRUE, Jsptr->GetType());
}

static void test_parse_false() {
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "false";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));
	EXPECT_EQ_INT(JVT_FALSE, Jsptr->GetType());
}

#define TEST_NUMBER(expect, json)\
    do {\
		string str = json;\
		JsParse mainJS;\
        shared_ptr<BaseJs> Jsptr;\
        EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));\
        EXPECT_EQ_INT(JVT_NUMBER, Jsptr->GetType());\
		double tarN = 0;\
		Jsptr->GetValue(tarN);\
        EXPECT_EQ_DOUBLE(expect, tarN);\
    } while(0)

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
		string str = json;\
		JsParse mainJS;\
        shared_ptr<BaseJs> Jsptr;\
        EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));\
        EXPECT_EQ_INT(JVT_STRING, Jsptr->GetType());\
		string tarS;\
		Jsptr->GetValue(tarS);\
        EXPECT_EQ_STRING(expect, tarS, tarS.length());\
    } while(0)

static void test_parse_string() {
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}


static void test_parse_array() {
	size_t i, j;
	JsParse mainJS; 
	shared_ptr<BaseJs> Jsptr; 
	vector<BaseJs> tvector;

	string str_1("[ ]");
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str_1));
	EXPECT_EQ_INT(JVT_ARRAY, Jsptr->GetType());
	Jsptr->GetValue(tvector);
	EXPECT_EQ_SIZE_T(0,tvector.size());

	Jsptr.reset();
	tvector.clear();
	string str_2("[ null , false , true , 123 , \"abc\" ]");
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str_2));
	EXPECT_EQ_INT(JVT_ARRAY, Jsptr->GetType());
	Jsptr->GetValue(tvector);
	EXPECT_EQ_SIZE_T(5, tvector.size());

	EXPECT_EQ_INT(JVT_NULL, tvector[0].GetType());
	EXPECT_EQ_INT(JVT_FALSE, tvector[1].GetType());
	EXPECT_EQ_INT(JVT_TRUE, tvector[2].GetType());
	EXPECT_EQ_INT(JVT_NUMBER, tvector[3].GetType());
	EXPECT_EQ_INT(JVT_STRING, tvector[4].GetType());

	double testD;
	tvector[3].GetValue(testD);
	EXPECT_EQ_DOUBLE(123.0,testD);

	string testS;
	tvector[4].GetValue(testS);
	EXPECT_EQ_STRING("abc", testS, testS.length());

	Jsptr.reset();
	tvector.clear();
	string str_3("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str_3));

	EXPECT_EQ_INT(JVT_ARRAY, Jsptr->GetType());
	Jsptr->GetValue(tvector);
	EXPECT_EQ_SIZE_T(4, tvector.size());

	for (i = 0; i < 4; i++) 
	{
		BaseJs* a = &tvector[i];
		EXPECT_EQ_INT(JVT_ARRAY, a->GetType());
		vector<BaseJs> temp_sub_v;
		a->GetValue(temp_sub_v);
		EXPECT_EQ_SIZE_T(i, temp_sub_v.size());
		for (j = 0; j < i; j++) 
		{
			BaseJs* e = &temp_sub_v[j];
			double temp_d_number;
			e->GetValue(temp_d_number);
			EXPECT_EQ_INT(JVT_NUMBER, e->GetType());
			EXPECT_EQ_DOUBLE((double)j, temp_d_number);
		}
	}

}

#define TEST_ERROR(error, json)\
    do {\
		string str = json;\
		JsParse mainJS;\
        shared_ptr<BaseJs> Jsptr;\
        EXPECT_EQ_INT(error, mainJS.JsonParse(Jsptr, str));\
    } while(0)

//EXPECT_EQ_INT(JVT_NULL, Jsptr->GetType());

static void test_parse_expect_value() {
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "?");

	/* invalid number */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "null x");

	/* invalid number */
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{1:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{true:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{false:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{null:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{[]:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{{}:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

int main()
{
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	//test_parse_object();

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();

	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();

	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();

	test_parse_miss_comma_or_square_bracket();
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();

	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

	return main_ret;
}