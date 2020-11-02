#ifndef SORBET_SUBSTITUTE_H
#define SORBET_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"
namespace sorbet::core {
class LazyGlobalSubstitution;
};
namespace sorbet::ast {
class Substitute {
public:
    static TreePtr run(core::MutableContext ctx, const core::GlobalSubstitution &subst, TreePtr what);

    static TreePtr run(core::MutableContext ctx, core::LazyGlobalSubstitution &subst, TreePtr what);
};
} // namespace sorbet::ast
#endif // SORBET_SUBSTITUTE_H
