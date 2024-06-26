/* SPDX-License-Identifier: Apache-2.0 */
/**
 * Copyright (c) 2022 Samsung Electronics Co., Ltd. All Rights Reserved.
 *
 * @file    model-dbus-impl.cc
 * @date    29 Jul 2022
 * @brief   DBus implementation for Model Interface
 * @see     https://github.com/nnstreamer/deviceMLOps.MLAgent
 * @author  Sangjung Woo <sangjung.woo@samsung.com>
 * @bug     No known bugs except for NYI items
 */

#include <errno.h>
#include <glib.h>

#include "common.h"
#include "dbus-interface.h"
#include "gdbus-util.h"
#include "log.h"
#include "model-dbus.h"
#include "modules.h"
#include "service-db-util.h"

static MachinelearningServiceModel *g_gdbus_instance = NULL;

/**
 * @brief Utility function to get the DBus proxy of Model interface.
 */
static MachinelearningServiceModel *
gdbus_get_model_instance (void)
{
  return machinelearning_service_model_skeleton_new ();
}

/**
 * @brief Utility function to release DBus proxy of Model interface.
 */
static void
gdbus_put_model_instance (MachinelearningServiceModel **instance)
{
  g_clear_object (instance);
}

/**
 * @brief The callback function of Register method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @param path The file path of target.
 * @param is_active The active status of target model.
 * @param description The description of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_register (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name, const gchar *path,
    const bool is_active, const gchar *description, const gchar *app_info)
{
  gint ret = 0;
  guint version = 0U;

  ret = svcdb_model_add (name, path, is_active, description, app_info, &version);
  machinelearning_service_model_complete_register (obj, invoc, version, ret);

  return TRUE;
}

/**
 * @brief The callback function of update description method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @param version The version of target model.
 * @param description The description of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_update_description (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name, const guint version,
    const gchar *description)
{
  gint ret = 0;

  ret = svcdb_model_update_description (name, version, description);
  machinelearning_service_model_complete_update_description (obj, invoc, ret);

  return TRUE;
}

/**
 * @brief The callback function of activate method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @param version The version of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_activate (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name, const guint version)
{
  gint ret = 0;

  ret = svcdb_model_activate (name, version);
  machinelearning_service_model_complete_activate (obj, invoc, ret);

  return TRUE;
}

/**
 * @brief The callback function of get method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_get (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name, const guint version)
{
  gint ret = 0;
  g_autofree gchar *model_info = NULL;

  ret = svcdb_model_get (name, version, &model_info);
  machinelearning_service_model_complete_get (obj, invoc, model_info, ret);

  return TRUE;
}

/**
 * @brief The callback function of get activated method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_get_activated (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name)
{
  gint ret = 0;
  g_autofree gchar *model_info = NULL;

  ret = svcdb_model_get_activated (name, &model_info);
  machinelearning_service_model_complete_get_activated (obj, invoc, model_info, ret);

  return TRUE;
}

/**
 * @brief The callback function of get all method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_get_all (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name)
{
  gint ret = 0;
  g_autofree gchar *model_info = NULL;

  ret = svcdb_model_get_all (name, &model_info);
  machinelearning_service_model_complete_get_all (obj, invoc, model_info, ret);

  return TRUE;
}

/**
 * @brief The callback function of delete method
 *
 * @param obj Proxy instance.
 * @param invoc Method invocation handle.
 * @param name The name of target model.
 * @param version The version of target model.
 * @param force If the force is set to @c TRUE, the target model will be forced to delete.
 * @return @c TRUE if the request is handled. FALSE if the service is not available.
 */
static gboolean
gdbus_cb_model_delete (MachinelearningServiceModel *obj,
    GDBusMethodInvocation *invoc, const gchar *name, const guint version, const gboolean force)
{
  gint ret = 0;

  ret = svcdb_model_delete (name, version, force);
  machinelearning_service_model_complete_delete (obj, invoc, ret);

  return TRUE;
}

/**
 * @brief Event handler list of Model interface
 */
static struct gdbus_signal_info handler_infos[] = {
  {
      .signal_name = DBUS_MODEL_I_HANDLER_REGISTER,
      .cb = G_CALLBACK (gdbus_cb_model_register),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_UPDATE_DESCRIPTION,
      .cb = G_CALLBACK (gdbus_cb_model_update_description),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_ACTIVATE,
      .cb = G_CALLBACK (gdbus_cb_model_activate),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_GET,
      .cb = G_CALLBACK (gdbus_cb_model_get),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_GET_ACTIVATED,
      .cb = G_CALLBACK (gdbus_cb_model_get_activated),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_GET_ALL,
      .cb = G_CALLBACK (gdbus_cb_model_get_all),
      .cb_data = NULL,
      .handler_id = 0,
  },
  {
      .signal_name = DBUS_MODEL_I_HANDLER_DELETE,
      .cb = G_CALLBACK (gdbus_cb_model_delete),
      .cb_data = NULL,
      .handler_id = 0,
  },
};

/**
 * @brief The callback function for probing Model Interface module.
 */
static int
probe_model_module (void *data)
{
  int ret = 0;

  ml_logd ("probe_model_module");

  g_gdbus_instance = gdbus_get_model_instance ();
  if (NULL == g_gdbus_instance) {
    ml_loge ("cannot get a dbus instance for the %s interface\n", DBUS_MODEL_INTERFACE);
    return -ENOSYS;
  }

  ret = gdbus_connect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);
  if (ret < 0) {
    ml_loge ("cannot register callbacks as the dbus method invocation handlers\n ret: %d", ret);
    ret = -ENOSYS;
    goto out;
  }

  ret = gdbus_export_interface (g_gdbus_instance, DBUS_MODEL_PATH);
  if (ret < 0) {
    ml_loge ("cannot export the dbus interface '%s' at the object path '%s'\n",
        DBUS_MODEL_INTERFACE, DBUS_MODEL_PATH);
    ret = -ENOSYS;
    goto out_disconnect;
  }

  return 0;

out_disconnect:
  gdbus_disconnect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);

out:
  gdbus_put_model_instance (&g_gdbus_instance);

  return ret;
}

/**
 * @brief The callback function for initializing Model Interface module.
 */
static void
init_model_module (void *data)
{
  gdbus_initialize ();
}

/**
 * @brief The callback function for exiting Model Interface module.
 */
static void
exit_model_module (void *data)
{
  gdbus_disconnect_signal (g_gdbus_instance, ARRAY_SIZE (handler_infos), handler_infos);
  gdbus_put_model_instance (&g_gdbus_instance);
}

static const struct module_ops model_ops = {
  .name = "model-interface",
  .probe = probe_model_module,
  .init = init_model_module,
  .exit = exit_model_module,
};

MODULE_OPS_REGISTER (&model_ops)
