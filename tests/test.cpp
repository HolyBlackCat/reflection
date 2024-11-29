#include <em/macros/meta/common.h>
#include <em/macros/meta/if_else.h>
#include <em/macros/meta/indexing.h>
#include <em/macros/meta/ranges.h>


#define EM_PUBLIC    EM_VERBATIM(access, public   , public:   )
#define EM_PRIVATE   EM_VERBATIM(access, private  , private:  )
#define EM_PROTECTED EM_VERBATIM(access, protected, protected:)
// `...` gets pasted in the class verbatim.
// `groupid` and `id` are single-word IDs that explain what `...` is. Can be anything.
#define EM_VERBATIM(groupid, id, ...) (_em_verbatim)(groupid,id,__VA_ARGS__)


// Those convert a sequence from a custom format to a standardized one.
// The output format is a list of variable declarations `([(type)], name [,init...])` or verbatim code blocks `((_em_verbatim), groupid, id, text...)`.

// Here the input format is a list `(type)(name [,init...])`.
#define DETAIL_EM_REFL_PREPROCESS_NORMAL(seq) EM_SEQ_GROUP2(seq)
// Here the input format is a list of `(name)`.
// The output has no type information and no initializers.
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES(seq) EM_END(DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_A seq)
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_A(name) DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_BODY(name) DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_B
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_B(name) DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_BODY(name) DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_A
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_A_END
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_B_END
#define DETAIL_EM_REFL_PREPROCESS_ONLYNAMES_BODY(name) (,name)

// Takes a list in the standard format (see `DETAIL_EM_REFL_PREPROCESS_...` above) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(p_type_, ...) EM_IF_COMMA((EM_CAT(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM_, EM_VA_AT(0, EM_IDENTITY p_type_))),(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM),(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL))(p_type_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM__em_verbatim ,
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL(p_type_, name_, .../*init_*/) ::std::type_identity_t<EM_IDENTITY p_type_> name_ EM_IF_EMPTY((__VA_ARGS__),({}),(=__VA_ARGS__));
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM(p_type_, groupid_, id_, .../*text_*/) __VA_ARGS__
