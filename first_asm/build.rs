fn main() {
    // Compile 'src/program.asm' into an object file and link it
    // The first argument is the output library name (e.g., "libasm_code.a")
    let _ = nasm_rs::compile_library("add_asm", &["src/add.asm"]);

    // Optional: Tell Cargo to rebuild if the asm file changes
    println!("cargo:rustc-link-lib=static=add_asm");
    println!("cargo:rerun-if-changed=src/add.asm");
}