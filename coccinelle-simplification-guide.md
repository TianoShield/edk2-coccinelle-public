# Simplifying Coccinelle Rules While Preserving Semantics

The goal of simplification is not fewer lines by itself. The goal is a smaller
semantic patch whose remaining structure still says exactly what makes the
rewrite valid.

## Preserve the Semantic Boundary

Before merging rules, write down the boundary that makes the transformation
correct. Keep that boundary explicit in the simplified rule.

Good boundary anchors include:

- The API or function family being changed.
- The struct type for a member call.
- The argument position being rewritten.
- The accepted constants or enum family.
- The enclosing construct, such as an `if` condition or `ASSERT` argument.

Do not hide these anchors behind broad `expression` or `identifier`
metavariables unless another constraint restores the same meaning.

## Simplification Moves

Use these moves for simplifying a semantic patch.

### Merge Near-identical Rules With Disjunctions

If rules differ only by one local syntax alternative, merge them into one rule
with a disjunction.

Before:

```cocci
@zero_eq@
expression E;
statement S;
@@
if (
- E == 0
+ IS_ZERO (E)
   ) S

@zero_ne@
expression E;
statement S;
@@
if (
- E != 0
+ !IS_ZERO (E)
   ) S
```

After:

```cocci
@zero_check@
expression E;
statement S;
@@
if (
(
- E == 0
+ IS_ZERO (E)
|
- E != 0
+ !IS_ZERO (E)
)
   ) S
```

The shared context stays in one place, so future edits are less likely to fix
one branch and miss the other.

### Replace Enumerated Constants With Constrained Identifiers

When many rules differ only by a regular naming pattern, match the pattern once
and generate the replacement.

Before:

```cocci
@flag_a@
@@
- OLD_FLAG_A
+ NEW_FLAG_A

@flag_b@
@@
- OLD_FLAG_B
+ NEW_FLAG_B

@flag_c@
@@
- OLD_FLAG_C
+ NEW_FLAG_C
```

After:

```cocci
@initialize:python@
@@

def to_new_flag(name):
    return name.replace("OLD_FLAG_", "NEW_FLAG_", 1)

@flags@
identifier old =~ "OLD_FLAG_(A|B|C)";
fresh identifier new = script:python(old) { to_new_flag(old) };
@@
- old
+ new
```

This preserves the spelling relationship directly instead of encoding it as a
series of unrelated replacements.

### Factor by the Shared Code Pattern

When two rules rewrite the same kind of function call, you can combine them into one rule. But the combined rule still needs to say exactly which object type and which methods it is allowed to match.

Before:

```cocci
@read_u32@
typedef DEVICE_IO;
DEVICE_IO *dev;
expression addr, out;
@@
dev->Read (
  dev,
- Width32,
+ DeviceWidth32,
  addr,
  out
  )

@write_u32@
typedef DEVICE_IO;
DEVICE_IO *dev;
expression addr, in;
@@
dev->Write (
  dev,
- Width32,
+ DeviceWidth32,
  addr,
  in
  )
```

After:

```cocci
@rw_u32@
typedef DEVICE_IO;
DEVICE_IO *dev;
identifier op =~ "^(Read|Write)$";
expression addr, value;
@@
dev->op (
  dev,
- Width32,
+ DeviceWidth32,
  addr,
  in
  )
```

The two original rules match different method names, Read and Write, but they have the same call shape; they can be merged into one rule. The merged rule still constraints the type of struct object.

### Use Dots for Irrelevant Structure

Use `...` for parameters, statements, or surrounding context that do not affect
correctness. Keep the facts that do affect correctness visible.

Before:

```cocci
@fixed_args@
typedef STATUS;
typedef VALUE;
identifier Name, Version, Value;
@@
STATUS
SetThing (
  char *Name,
  int Version,
- VALUE Value
+ VALUE *Value
  )
{
  ...
}
```

After:

```cocci
@essential_arg@
typedef STATUS;
typedef VALUE;
identifier Value;
@@
STATUS
SetThing (
  ...,
- VALUE Value
+ VALUE *Value
  )
{
  ...
}
```

The function name, return type, and changed parameter type remain concrete. The
unrelated leading arguments no longer overfit the rule.

### Combine Related Rewrites in One Rule

When two rewrites are part of the same logical change, keep them in the same Coccinelle rule.

Before:

```cocci
@param_to_pointer@
typedef VALUE;
identifier v;
@@
void Update (
- VALUE v
+ VALUE *v
  )
{
  ...
}

@member_to_arrow@
typedef VALUE;
identifier v, field;
@@
void Update (
  VALUE *v
  )
{
  <...
- v.field
+ v->field
  ...>
}
```

After:

```cocci
@value_pointer_update@
typedef VALUE;
identifier v, field;
@@
void Update (
- VALUE v
+ VALUE *v
  )
{
  <...
- v.field
+ v->field
  ...>
}
```

In the same Update function where the parameter v is changed from `VALUE` to `VALUE *`, also update uses of `v.field` inside that body to `v->field`.
The simplification also makes the rule safer: the `v.field` to `v->field` rewrite only happens in the function body where v was actually converted into a pointer.

### Use Isomorphisms
Rely on standard isomorphisms for equivalent forms instead of spelling out
local alternatives. For example, the standard isomorphisms already cover
common NULL-test shapes such as `if (!y)`, `if (y == NULL)`, and
`if (NULL == y)`. Do not add a disjunction merely to enumerate those forms.

The same applies to casts covered by the `drop_cast` isomorphism. Write the
canonical casted form and let the isomorphism also match the uncasted form.

Before:

```cocci
@cast_or_uncast@
identifier id;
type T;
@@
(
- Use (id)
+ UseNew (id)
|
- Use ((T) id)
+ UseNew (id)
)
```

After:

```cocci
@casted_expression@
identifier id;
type T;
@@
- Use ((T) id)
+ UseNew (id)
```

Use explicit alternatives only when the forms are semantically different for
the transformation, or when a standard isomorphism would make the rule broader
than the intended pattern. In that case, keep the boundary visible in the rule
or disable the specific isomorphism for that rule, for example
`@rule disable drop_cast@`.
