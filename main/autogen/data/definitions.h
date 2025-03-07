#ifndef AUTOGEN_DEFINITIONS_H
#define AUTOGEN_DEFINITIONS_H

#include "ast/ast.h"

namespace sorbet::autogen {

// The types defined here are simplified views of class and constant definitions in a Ruby codebase, which we use to
// create static output information and autoloader files

// A `QualifiedName` is a vector of namerefs that includes the fully-qualified name as well as an optional package name
// (if we're running in `--stripe-packages` mode)
struct QualifiedName {
    std::vector<core::NameRef> nameParts;
    std::optional<core::NameRef> package;

    static QualifiedName fromFullName(std::vector<core::NameRef> fullName);

    bool empty() const {
        return nameParts.empty();
    }

    u4 size() const {
        return nameParts.size();
    }

    bool operator==(const QualifiedName &rhs) const {
        return nameParts == rhs.nameParts && package == rhs.package;
    }

    core::NameRef name() const {
        return nameParts.back();
    }

    std::string show(const core::GlobalState &gs) const;
    std::string join(const core::GlobalState &gs, std::string_view sep) const;
};

const u4 NONE_ID = (u4)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct DefinitionRef;
struct ReferenceRef;

struct AutoloaderConfig;
struct NamedDefinition;
class DefTree;

enum class ClassKind { Class, Module };

// A reference to a specific `Definition` inside of a `ParsedFile`.
struct DefinitionRef {
    u4 _id;

    DefinitionRef() : _id(NONE_ID){};
    DefinitionRef(u4 id) : _id(id) {}

    u4 id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Definition &data(const ParsedFile &pf) const;
};

// A reference to a specific `Reference` inside of a `ParsedFile`.
struct ReferenceRef {
    u4 _id;
    ReferenceRef() : _id(NONE_ID){};
    ReferenceRef(u4 id) : _id(id) {}

    u4 id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Reference &data(const ParsedFile &pf) const;
};

// A constant definition---a class, module, constant definition, or constant alias---along with relevant metadata
struct Definition {
    enum class Type : u8 { Module, Class, Casgn, Alias };

    // the reference to this definition. Once `AutogenWalk` is completed and a full `ParsedFile` has been created, it
    // should always be the case that
    //   definition.id.data(pf) == definition
    DefinitionRef id;

    // is this a class, module, constant, or alias
    Type type;
    // does this define behavior? (i.e. is it a casgn or class, not simply a namespace?)
    bool defines_behavior;
    // does it contain other things?
    bool is_empty;

    // if this is a class, then `parent_ref` will be the reference to the parent class; otherwise it will be undefined
    ReferenceRef parent_ref;

    // if this is an alias, then `aliased_ref` will be the reference it is an alias for; otherwise it will be undefined
    ReferenceRef aliased_ref;

    // which ref is the one that corresponds to this definition? I (gdritter) _believe_ that this will always be defined
    // once `AutogenWalk` has completed; please update this comment if that ever turns out to be false
    ReferenceRef defining_ref;
};

// A `Reference` corresponds to a simple use of a constant name in a file. After a `ParsedFile` has been created, every
// constant use should have a `Reference` corresponding to it _unless_ it appears in a `keep_for_ide` call.
struct Reference {
    // the reference to this reference. Once `AutogenWalk` is completed and a full `ParsedFile` has been created, it
    // should always be the case that
    //   reference.id.data(pf) == reference
    ReferenceRef id;

    // In which class or module was this reference used?
    DefinitionRef scope;

    // its full qualified name
    QualifiedName name;
    // the full nesting of this constant. If it's a constant resolved from the root, this will be an empty vector
    std::vector<DefinitionRef> nesting;
    // the resolved name iff we have it from Sorbet
    QualifiedName resolved;

    // loc and definitionLoc will differ if this is a defining ref, otherwise they will be the same
    core::Loc loc;
    core::Loc definitionLoc;

    // this is always true
    // TODO(gdritter): delete this, of course
    bool is_resolved_statically;
    // `true` if this is the appearance of the constant name associated with a definition: i.e. the name of a class or
    // module or the LHS of a casgn
    bool is_defining_ref;

    // Iff this is a class, then this will be `ClassKind::Class`, otherwise `ClassKind::Module`
    ClassKind parentKind = ClassKind::Module;

    // If this is a ref used in an `include` or `extend`, then this will point to the definition of the class in which
    // this is being `include`d or `extend`ed
    DefinitionRef parent_of;
};

// A `ParsedFile` contains all the `Definition`s and `References` used in a particular file
struct ParsedFile {
    friend class MsgpackWriter;

    // the original file AST from Sorbet
    ast::ParsedFile tree;
    // the checksum of this file
    u4 cksum;
    // the path on disk to this file
    std::string path;
    // every statically-known constant defined by this file
    std::vector<Definition> defs;
    // every static constant usage in this file
    std::vector<Reference> refs;
    // every required gem in this file
    std::vector<core::NameRef> requires;

    std::string toString(const core::GlobalState &gs) const;
    std::string toMsgpack(core::Context ctx, int version);
    std::vector<core::NameRef> showFullName(const core::GlobalState &gs, DefinitionRef id) const;
    QualifiedName showQualifiedName(const core::GlobalState &gs, DefinitionRef id) const;
    std::vector<std::string> listAllClasses(core::Context ctx);
};

// A `Package` represents Autogen's view of a package file
struct Package {
    // the original file AST from Sorbet
    ast::ParsedFile tree;

    std::vector<core::NameRef> package;
    std::vector<QualifiedName> imports;
    std::vector<QualifiedName> exports;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_DEFINITIONS_H
