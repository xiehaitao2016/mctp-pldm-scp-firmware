#include <mod_ras_handler.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

const unsigned int interrupt_ids[] = {21,22,23,24,25,26,27,29};

static const struct fwk_element ras_element_table[] = {
    [0] = { .name = "RD-V3-ras",
            .data = &((struct mod_ras_config){
                .interrupt_count = FWK_ARRAY_SIZE(interrupt_ids),
                .interrupt_id = interrupt_ids,
          }),
     },
     [1] = { 0 },
};

static const struct fwk_element *ras_get_element_table(fwk_id_t module_id)
{
    return ras_element_table;
}

const struct fwk_module_config config_ras_handler = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(ras_get_element_table),
};

