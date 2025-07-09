/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI pin control protocol HAL support.
 */

#ifndef MOD_PINCTRL_H
#define MOD_PINCTRL_H

#include <mod_pinctrl_drv.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModulePinctrl pinctrl
 * \{
 */

/*!
 * \brief Selector to differentiate between pin, group or functionality.
 */
enum mod_pinctrl_selector {
    /*! Pin selector id */
    MOD_PINCTRL_SELECTOR_PIN = 0,

    /*! Group selector id */
    MOD_PINCTRL_SELECTOR_GROUP,

    /*! Functionality selector id */
    MOD_PINCTRL_SELECTOR_FUNCTION,
};

/*!
 * \brief Function type to define the functins table.
 */
struct mod_pinctrl_functionality {
    /*! Pointer to NULL terminated string for name of functionality */
    char *name;

    /*! Attribute to mark this functionality as GPIO */
    bool is_gpio;

    /*! Attribute to mark this functionality as used only for pins */
    bool is_pin_only;
};

/*!
 * \brief Group type to define the Groups table.
 */
struct mod_pinctrl_group {
    /*! Pointer to NULL terminated string for name of functionality */
    char *name;

    /*! Functionality associated to the group */
    uint16_t functionality_id;

    /*! Pointer to the pins associated to the group */
    const uint16_t *pins;

    /*! Total number of pins associated to the group */
    uint8_t num_pins;
};

/*!
 * \brief Attributes type to define the pin, group or functionality
 *      characteristics to be returned to SCMI.
 */
struct mod_pinctrl_attributes {
    /*! Pointer to NULL terminated string for the name*/
    char *name;

    /*! Number of elements to be returned to SCMI*/
    uint16_t number_of_elements;

    /*! Attribute to mark this functionality as used only for pins */
    bool is_pin_only_functionality;

    /*! Attribute to mark this functionality as GPIO */
    bool is_gpio_functionality;
};

/*!
 * \brief Attributes type to define the total numbers of pin, and groups and
 * functionalities counts.
 */
struct mod_pinctrl_protocol_attributes {
    /*! Number of pinctrl pins */
    uint16_t number_of_pins;

    /*! Number of pinctrl groups */
    uint16_t number_of_groups;

    /*! Number of pinctrl functionalities */
    uint16_t number_of_functionalities;
};

/*!
 * \brief Module configuration.
 */
struct mod_pinctrl_config {
    /*! Pointer to the table of functionality descriptors. */
    struct mod_pinctrl_functionality *functionalities_table;

    /*! Number of functionalities */
    uint16_t functionality_table_count;

    /*! Pointer to the table of group descriptors. */
    struct mod_pinctrl_group *groups_table;

    /*! Number of groups that will be associated to pins */
    uint16_t group_table_count;

    /*! Driver identifier */
    fwk_id_t driver_id;

    /*! Driver API identifier */
    fwk_id_t driver_api_id;
};

/*!
 * \brief mod pin control APIs.
 *
 * \details APIs exported by pin control.
 */
enum mod_pinctrl_api_idx {

    /*! Index for the pin control API */
    MOD_PIN_CONTROL_API,

    /*! Number of APIs */
    MOD_PIN_CONTROL_API_COUNT
};

struct mod_pinctrl_api {
    /*!
     * \brief Get attributes of pin, group or functionality.
     *
     * \param[in] index Identifier for the pin, group, or functionality.
     * \param[in] flags Selector: Whether the identifier field selects
     *      a pin, a group, or a functionality.
     *      0 - pin
     *      1 - group
     *      2 - functionality
     * \param[out] attributes respond to get attribute request
     *      number_of_elements: total number of elements.
     *      is_pin_only_functionality: enum group_pin_serving_t.
     *      is_gpio_functionality: enum gpio_functionality_t.
     *      name: Null-terminated ASCII string.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE if the identifier field pertains to a
     *      non-existent pin, group, or functionality.
     */
    int (*get_attributes)(
        uint16_t index,
        enum mod_pinctrl_selector flags,
        struct mod_pinctrl_attributes *attributes);

    /*!
     * \brief Get attributes of pin, group or functionality.
     *
     * \param[out] protocol_attributes return protocol attributes
     *      Number of pin groups.
     *      Number of pins.
     *      Reserved, must be zero.
     *      Number of functionality.
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*get_protocol_attributes)(
        struct mod_pinctrl_protocol_attributes *protocol_attributes);

    /*!
     * \brief Get pin associated with a group, or
     *      Get group which can enable a functionality, or
     *      Get pin which can enable a single-pin functionality.
     * \param[in] index Identifier for the group, or functionality.
     * \param[in] flags Selector: Whether the identifier field selects
     *      group, or functionality.
     *      1 - group
     *      2 - functionality
     * \param[in] first_index the index of the object {pin, group} to be
     *      returned
     * \param[out] object_id the returned object.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE index >= max number of pins associated with this
     *      functionality or group.
     * \retval ::FWK_E_PARAM the flags is pin.
     */
    int (*get_list_associations)(
        uint16_t index,
        enum mod_pinctrl_selector flags,
        uint16_t first_index,
        uint16_t *object_id);

    /*!
     * \brief Get the total number of associated pins or groups to index.
     * \param[in] index Identifier for the group, or functionality.
     * \param[in] flags Selector: Whether the identifier field selects
     *      group, or functionality.
     *      1 - group
     *      2 - functionality
     * \param[in] total_count the total number of associations to the index
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM index invalid index or flags == pin
     */
    int (*get_total_number_of_associations)(
        uint16_t index,
        enum mod_pinctrl_selector flags,
        uint16_t *total_count);

    /*!
     * \brief Get pin or group specific Configuration.
     * \param[in] index Identifier for the pin, or group.
     * \param[in] flags Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[in] config_type Configuration type to be retun its value.
     * \param[out] config_value Configuration value.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE index >= max number of pins or groups.
     * \retval ::FWK_E_PARAM the flags isn't 0 or 1.
     */
    int (*get_configuration_value_from_type)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        enum mod_pinctrl_drv_configuration_type config_type,
        uint32_t *config_value);

    /*!
     * \brief Get pin or group specific Configuration.
     * \param[in] index Identifier for the pin, or group.
     * \param[in] flags Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[in] configration_index
     * \param[out] config Configuration type to be retun its value.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE configration_index > total number of configurtions.
     * \retval ::FWK_E_PARAM the flags isn't pin or group. or index >= max
     *      number of pins or groups.
     */
    int (*get_configuration)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        uint16_t configration_index,
        struct mod_pinctrl_drv_pin_configuration *config);

    /*!
     * \brief Get the total number of configurations of pin or group
     * \param[in] index Identifier for the pin, or group.
     * \param[in] flags Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[out] number_of_configurations the umber of configurations
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM the flags == function. or index >= max
     *      number of pins or groups.
     */
    int (*get_total_number_of_configurations)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        uint16_t *number_of_configurations);

    /*!
     * \brief get current pin or group enabled functionality.
     *
     * \param[in] index pin or group identifier.
     * \param[in] flag Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[out] functionality_id current enabled functionality.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins or the
     *      functionality_id is not allowed for this pin_id.
     * \retval ::FWK_E_PARAM the flag isn't pin or group.
     */
    int (*get_current_associated_functionality)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        uint16_t *functionality_id);

    /*!
     * \brief set pin or group configuration.
     *
     * \param[in] index pin or group identifier.
     * \param[in] flag Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[in] config configuration to be set.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins or the
     *      functionality_id is not allowed for this pin_id.
     * \retval ::FWK_E_PARAM the flag isn't pin or group.
     */
    int (*set_configuration)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        const struct mod_pinctrl_drv_pin_configuration *config);

    /*!
     * \brief Set pin or group functionality.
     *
     * \param[in] index pin or group identifier.
     * \param[in] flag Selector: Whether the identifier field selects
     *      a pin or group.
     *      0 - pin
     *      1 - group
     * \param[in] functionality_id the fucntionality to be set.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_RANGE pin_id >= max number of pins or the
     *      functionality_id is not allowed for this pin_id.
     * \retval ::FWK_E_PARAM the flag isn't pin or group.
     */
    int (*set_functionality)(
        uint16_t index,
        enum mod_pinctrl_selector flag,
        uint16_t functionality_id);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PINCTRL_H */
