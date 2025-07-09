/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI pin control protocol DRV support APIs.
 */

#ifndef MOD_PINCTRL_DRV_H
#define MOD_PINCTRL_DRV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModulePinctrl_drv pinctrl_drv
 * \{
 */

/*!
 * \brief Pre-defined pin configurations types from SCMI.
 */
enum mod_pinctrl_drv_configuration_type {
    DEFAULT = 0,
    BIAS_BUS_HOLD,
    BIAS_DISABLE,
    BIAS_HIGH_IMPEDANCE,
    BIAS_PULL_UP,
    BIAS_PULL_DEFAULT,
    BIAS_PULL_DOWN,
    DRIVE_OPEN_DRAIN,
    DRIVE_OPEN_SOURCE,
    DRIVE_PUSH_PULL,
    DRIVE_STRENGTH,
    INPUT_DEBOUNCE,
    INPUT_MODE,
    PULL_MODE,
    INPUT_VALUE,
    INPUT_SCHMITT,
    LOW_POWER_MODE,
    OUTPUT_MODE,
    OUTPUT_VALUE,
    POWER_SOURCE,
    SLEW_RATE,
};

/*!
 * \brief Pin configuration type to link between configurations types and its
 *      value.
 */
struct mod_pinctrl_drv_pin_configuration {
    /*! Configuration type */
    enum mod_pinctrl_drv_configuration_type config_type;

    /*! Configuration value */
    uint32_t config_value;
};

/*!
 * \brief Pin type to define all pin characteristics and pins table.
 */
struct mod_pinctrl_drv_pin {
    /*! Pointer to NULL terminated string for name of pin */
    const char *name;

    /*! Current pin functionality */
    uint16_t functionality;

    /*! Allowed pin functionalities to choose from */
    const uint16_t *allowed_functionalities;

    /*! Number of allowed pin functionalities to choose from */
    const size_t num_allowed_functionalities;

    /*! Table of pin configurations */
    struct mod_pinctrl_drv_pin_configuration *configuration;

    /*! Number of pin configurations */
    const size_t num_configuration;
};

/*!
 * \brief Module configuration.
 */
struct mod_pinctrl_drv_config {
    /*! \brief Pointer to the table of pins descriptors. */
    struct mod_pinctrl_drv_pin *pins;

    /*! Number of pins */
    size_t pin_count;
};

/*!
 * \brief mod pin control attributes APIs.
 *
 * \details APIs exported by pin control driver.
 */
enum mod_pinctrl_drv_api_idx {
    /*! Index for the pin control attributes API */
    MOD_PINCTRL_API_IDX,

    /*! Number of APIs */
    MOD_PINCTRL_API_COUNT
};

struct mod_pinctrl_drv_api {
    /*!
     * \brief Get the total number of pins that can support functionality_id.
     *
     * \param[in] functionality_id functionality identintifier.
     * \param[out] number_of_pins total number of pins which can support
     *      functionality_id.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM Number of pins pointer is NULL.
     */
    int (*get_number_pins_can_support_functionalityality)(
        uint16_t functionality_id,
        uint16_t *number_of_pins);

    /*!
     * \brief Get pin name and name_type.
     *
     * \param[in] pin_id pin identintifier.
     * \param[out] name_type name type may be normal or extended.
     * \param[out] name name string terminated with null char \0.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins.
     */
    int (*get_pin_name)(uint16_t pin_id, char **pin_name, bool *name_type);

    /*!
     * \brief get pin_id that can enable a single-pin functionality.
     *
     * \param[in] functionality_id functionality identintifier.
     * \param[in] index Index of the pin to be described
     *      in the return pin_id.
     * \param[out] pin_id pin id which can enable functionality_id.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE index >= max number of pins associated
     *      with this functionality.
     */
    int (*get_pin_id_associated_with_functionality)(
        uint16_t functionality_id,
        uint16_t index,
        uint16_t *pin_id);

    /*!
     * \brief The current functionality selected to be enabled by the pin.
     *
     * \param[in] pin_id pin identintifier.
     * \param[out] selected_functionality currently selected/enabled
     * functionality. \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins.
     */
    int (*get_pin_current_selected_functionality)(
        uint16_t pin_id,
        uint16_t *selected_functionality);

    /*!
     * \brief get pin configuration.
     *
     * \param[in] pin_id pin identifier.
     * \param[in] pin_config Configuration type and value.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins.
     * \retval ::FWK_E_PARAM invalid config_type.
     */
    int (*get_pin_configuration)(
        uint16_t pin_id,
        const struct mod_pinctrl_drv_pin_configuration *pin_config);

    /*!
     * \brief get pin functionality.
     *
     * \param[in] pin_id pin identifier.
     * \param[out] functionality_id functionality to be set.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins or the
     *      functionality_id is not allowed for this pin_id.
     */
    int (*get_pin_functionality)(uint16_t pin_id, uint16_t *functionality_id);

    /*!
     * \brief set pin functionality.
     *
     * \param[in] pin_id pin identifier.
     * \param[in] functionality_id functionality to be set.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins or the
     *      functionality_id
     * \retval ::FWK_E_PARAM functionality_id is not supported by the pin. is
     *      not allowed for this pin_id.
     */
    int (*set_pin_functionality)(uint16_t pin_id, uint16_t functionality_id);

    /*!
     * \brief set pin configuration.
     *
     * \param[in] pin_id pin identifier.
     * \param[in] pin_config Configuration type and value.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE: if pin_id field does not point to a valid pin.
     * \retval ::FWK_E_PARAM:: if the config_type specify incorrect or
     *      illegal values.
     */
    int (*set_pin_configuration)(
        uint16_t pin_id,
        const struct mod_pinctrl_drv_pin_configuration *pin_config);

    /*!
     * \brief get the total number of pins.
     *
     * \retval ::total number of pins.
     */
    int (*get_total_number_of_pins)(int *number_of_pins);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PINCTRL_DRV_H */
