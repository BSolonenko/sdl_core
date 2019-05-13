/*
 * Copyright (c) 2016, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SRC_COMPONENTS_INCLUDE_APPLICATION_MANAGER_STATE_CONTROLLER_H_
#define SRC_COMPONENTS_INCLUDE_APPLICATION_MANAGER_STATE_CONTROLLER_H_

#include "application_manager/application.h"
#include "application_manager/application_manager.h"
#include "application_manager/request_controller_settings.h"
#include "stdint.h"

namespace application_manager {
class StateController {
 public:
  /**
   * @brief SetRegularState setup regular hmi state, that  will appear if
   * no specific events are active
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param state state of new regular state
   */
  virtual void SetRegularState(ApplicationSharedPtr app,
                               const WindowID window_id,
                               HmiStatePtr state,
                               const bool SendActivateApp) = 0;

  /**
   * @brief SetRegularState Change regular hmi level and audio state
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param hmi_level of new regular state
   * @param audio_state of new regular state
   * @paran video_state of new regular state
   * @param SendActivateApp: if true, ActivateAppRequest will be sent on HMI
   */
  virtual void SetRegularState(
      ApplicationSharedPtr app,
      const WindowID window_id,
      const mobile_apis::HMILevel::eType hmi_level,
      const mobile_apis::AudioStreamingState::eType audio_state,
      const mobile_apis::VideoStreamingState::eType video_state,
      const bool SendActivateApp) = 0;

  /**
   * @brief SetRegularState Change regular hmi level
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param hmi_level of new regular state
   * @param SendActivateApp: if true, ActivateAppRequest will be sent on HMI
   */
  virtual void SetRegularState(ApplicationSharedPtr app,
                               const WindowID window_id,
                               const mobile_apis::HMILevel::eType hmi_level,
                               const bool SendActivateApp) = 0;

  /**
   * @brief SetRegularState Change regular hmi level, audio state and system
   * context
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param hmi_level of new regular state
   * @param audio_state of new regular state
   * @param video_state of new regular state
   * @param system_context of new regular state
   * @param SendActivateApp: if true, ActivateAppRequest will be sent on HMI
   */
  virtual void SetRegularState(
      ApplicationSharedPtr app,
      const WindowID window_id,
      const mobile_apis::HMILevel::eType hmi_level,
      const mobile_apis::AudioStreamingState::eType audio_state,
      const mobile_apis::VideoStreamingState::eType video_state,
      const mobile_apis::SystemContext::eType system_context,
      const bool SendActivateApp) = 0;

  /**
   * @brief SetRegularState Sets regular state with new hmi level
   * to application
   * @param app appication to setup regular state
   * @param window_id id of applicaion's window to apply HMI state
   * @param hmi_level new hmi level for application
   */
  virtual void SetRegularState(
      ApplicationSharedPtr app,
      const WindowID window_id,
      const mobile_apis::HMILevel::eType hmi_level) = 0;

  /**
   * @brief SetRegularState Change regular audio state
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param audio_state of new regular state
   * @param video_state of new regular state
   */
  virtual void SetRegularState(
      ApplicationSharedPtr app,
      const WindowID window_id,
      const mobile_apis::AudioStreamingState::eType audio_state,
      const mobile_apis::VideoStreamingState::eType video_state) = 0;

  /**
   * @brief SetRegularState Change regular  system context
   * @param app appication to setup regular State
   * @param window_id id of applicaion's window to apply HMI state
   * @param system_context of new regular state
   */
  virtual void SetRegularState(
      ApplicationSharedPtr app,
      const WindowID window_id,
      const mobile_apis::SystemContext::eType system_context) = 0;

  /**
   * @brief SetRegularState Sets new regular state to application
   * @param app appication to setup regular state
   * @param window_id id of applicaion's window to apply HMI state
   * @param state new hmi state for application
   */
  virtual void SetRegularState(ApplicationSharedPtr app,
                               const WindowID window_id,
                               HmiStatePtr state) = 0;

  /**
   * @brief Sets default application state and apply currently active HMI states
   * on application registration
   * @param app application to apply states
   * @param default_level default HMI level
   */
  virtual void OnApplicationRegistered(
      ApplicationSharedPtr app,
      const mobile_apis::HMILevel::eType default_level) = 0;

  virtual int64_t SendBCActivateApp(ApplicationConstSharedPtr app,
                                    hmi_apis::Common_HMILevel::eType level,
                                    bool send_policy_priority) = 0;
  /**
   * @brief OnVideoStreamingStarted process video streaming started
   * @param app projection or navigation application starting streaming
   */
  virtual void OnVideoStreamingStarted(ApplicationConstSharedPtr app) = 0;

  /**
   * @brief OnVideoStreamingStopped process video streaming stopped
   * @param app projection or navigation application stopping streaming
   */
  virtual void OnVideoStreamingStopped(ApplicationConstSharedPtr app) = 0;

  /**
   * @brief OnStateChanged send HMIStatusNotification if needed
   * @param app application
   * @param window_id id of applicaion's window to apply HMI state
   * @param old_state state before change
   * @param new_state state after change
   */
  virtual void OnStateChanged(ApplicationSharedPtr app,
                              const WindowID window_id,
                              HmiStatePtr old_state,
                              HmiStatePtr new_state) = 0;

  /**
   * @brief Checks activity of Deactivate HMI state.
   * @return Returns TRUE if deactivate HMI state is active, otherwise returns
   * FALSE.
   */
  virtual bool IsStateActive(HmiState::StateID state_id) const = 0;
};

}  // namespace application_manager

#endif  // SRC_COMPONENTS_INCLUDE_APPLICATION_MANAGER_STATE_CONTROLLER_H_
