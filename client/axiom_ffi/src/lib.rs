use axiom_core;

#[cfg_attr(cfg, no_mangle)]
pub unsafe extern "C" fn axiom_create_game() {
    axiom_core::create_game();
}
