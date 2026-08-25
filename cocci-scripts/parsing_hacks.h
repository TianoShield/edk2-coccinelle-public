/* This file contains parsing hacks for Coccinelle (spatch), to make it happy with some of EDK2
 * macros - it is intended to be used with the --macro-file-builtins option for spatch.
 *
 * Coccinelle's macro support is somewhat limited and the parser trips over some of EDK2 macros.
 * In most cases this doesn't really matter, as the parsing errors are silently ignored, but there are
 * special cases in which the parser incorrectly infers information that then causes issues in valid code
 * later down the line.
 *
 * Inspired by a similarly named file [0] from the Coccinelle sources, the original builtin macros [1],
 * and a similarly named file [2] from systemd.
 *
 * [0] https://github.com/coccinelle/coccinelle/blob/master/parsing_c/parsing_hacks.ml
 * [1] https://github.com/coccinelle/coccinelle/blob/master/standard.h
 * [2] https://github.com/systemd/systemd/blob/main/coccinelle/parsing_hacks.h
 *
 */

/* Drop these modifiers for data types and calling conventions. Otherwise, spatch can choke on
 * declarations such as `IN UINT64 Length OPTIONAL`, `IN EFI_PCI_IO_PROTOCOL *PciIo;`, and
 * function pointer typedefs using EFIAPI. */
#define IN
#define OUT
#define OPTIONAL
#define EFIAPI
#define STATIC static

/* Base.h qualifier macros. Without CONST, a local declaration such as `CONST UINT8 *Src;` is two
 * unknown type-ish tokens in a row, which fails to parse; the resulting recovery expands file-local
 * #defines in the whole enclosing function, hiding their uses from semantic patches (seen in
 * OvmfPkg/Sec/SecMain.c SecCoreStartupWithStack()). */
#define CONST const
#define VOLATILE volatile
