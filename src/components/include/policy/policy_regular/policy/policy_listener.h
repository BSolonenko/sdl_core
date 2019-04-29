/*
 Copyright (c) 2016, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SRC_COMPONENTS_INCLUDE_POLICY_POLICY_REGULAR_POLICY_POLICY_LISTENER_H_
#define SRC_COMPONENTS_INCLUDE_POLICY_POLICY_REGULAR_POLICY_POLICY_LISTENER_H_

#include <queue>

#include "policy/policy_types.h"
#include "utils/custom_string.h"

namespace policy {

namespace custom_str = utils::custom_string;

class PolicyListener {
 public:
  virtual ~PolicyListener() {}
  virtual void OnPermissionsUpdated(const std::string& policy_app_id,
                                    const Permissions& permissions,
                                    const policy::HMILevel& default_hmi) = 0;
  virtual void OnPermissionsUpdated(const std::string& policy_app_id,
                                    const Permissions& permissions) = 0;
  virtual void OnPendingPermissionChange(const std::string& policy_app_id) = 0;
  virtual void OnUpdateStatusChanged(const std::string&) = 0;
  virtual std::string OnCurrentDeviceIdUpdateRequired(
      const std::string& policy_app_id) = 0;
  virtual void OnSystemInfoUpdateRequired() = 0;
  virtual custom_str::CustomString GetAppName(
      const std::string& policy_app_id) = 0;
  virtual void OnUpdateHMIAppType(
      std::map<std::string, StringArray> app_hmi_types) = 0;

  /**
   * @brief CanUpdate allows to find active application
   * and check whether related device consented.
   *
   * @return true if there are at least one application has been registered
   * with consented device.
   */
  virtual bool CanUpdate() = 0;

  /**
   * @brief OnSnapshotCreated the notification which will be sent
   * when snapshot for PTU has been created.
   *
   * @param pt_string the snapshot
   *
   * @param retry_seconds retry sequence timeouts.
   *
   * @param timeout_exceed timeout.
   */
  virtual void OnSnapshotCreated(const BinaryMessage& pt_string) = 0;

  /**
   * @brief Make appropriate changes for related applications permissions and
   * notify them, if it possible
   * @param device_id Unique device id, which consent had been changed
   * @param device_consent Device consent, which is done by user
   */
  virtual void OnDeviceConsentChanged(const std::string& device_id,
                                      bool is_allowed) = 0;

  /**
   * @brief Sends OnAppPermissionsChanged notification to HMI
   * @param permissions contains parameter for OnAppPermisionChanged
   * @param policy_app_id contains policy application id
   */
  virtual void SendOnAppPermissionsChanged(
      const AppPermissions& permissions,
      const std::string& policy_app_id) const = 0;

  /**
   * @brief GetAvailableApps allows to obtain list of registered applications.
   */
  virtual void GetAvailableApps(std::queue<std::string>&) = 0;

  /**
   * @brief OnCertificateUpdated the callback which signals if certificate field
   * has been updated during PTU
   *
   * @param certificate_data the value of the updated field.
   */
  virtual void OnCertificateUpdated(const std::string& certificate_data) = 0;

  /**
   * @brief OnAuthTokenUpdated the callback which signals if an app's auth token
   * field has been updated during a PTU
   *
   * @param policy_app_id the policy app id tied to the updated field.
   * @param auth_token the value of the updated field.
   */
  virtual void OnAuthTokenUpdated(const std::string& policy_app_id,
                                  const std::string& auth_token) = 0;

  /**
   * Gets devices ids by policy application id
   * @param policy_app_id
   * @return list devices ids
   */
  virtual std::vector<std::string> GetDevicesIds(
      const std::string& policy_app_id) = 0;

  /**
   * Notifies about changing HMI level
   * @param device_id unique identifier of device
   * @param policy_app_id unique identifier of application in policy
   * @param hmi_level default HMI level for this application
   */
  virtual void OnUpdateHMILevel(const std::string& device_id,
                                const std::string& policy_app_id,
                                const std::string& hmi_level) = 0;

  /**
   * @brief Notifies Remote apps about change in permissions
   * @param device_id Device on which app is running
   * @param application_id ID of app whose permissions are changed
   */
  virtual void OnRemoteAppPermissionsChanged(
      const std::string& device_id, const std::string& application_id) = 0;

  /**
   * Notifies about changing HMI status
   * @param device_id unique identifier of device
   * @param policy_app_id unique identifier of application in policy
   * @param hmi_level default HMI level for this application
   */
  virtual void OnUpdateHMIStatus(const std::string& device_id,
                                 const std::string& policy_app_id,
                                 const std::string& hmi_level) = 0;
};
}  //  namespace policy
#endif  // SRC_COMPONENTS_INCLUDE_POLICY_POLICY_REGULAR_POLICY_POLICY_LISTENER_H_
