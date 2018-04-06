/***********************************************************************************************************************
 *
 * Copyright (c) 2015, ABB Schweiz AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 *
 *    * Redistributions of source code must retain the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer.
 *    * Redistributions in binary form must reproduce the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer in the documentation
 *      and/or other materials provided with the
 *      distribution.
 *    * Neither the name of ABB nor the names of its
 *      contributors may be used to endorse or promote
 *      products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************************************************************
 */

#ifndef RWS_INTERFACE_H
#define RWS_INTERFACE_H

#include "rws_client.h"

namespace abb
{
namespace rws
{
/**
 * \brief A class for wrapping a Robot Web Services (RWS) client in a more user friendly interface.
 */
class RWSInterface
{
public:
  /**
   * \brief A struct for a tri value bool.
   */
  struct TriBool
  {
    /**
     * \brief An enum for the different accepted values.
     */
    enum Values
    {
      UNKNOWN_VALUE, ///< Unknown value. E.g. in case of communication failure.
      TRUE_VALUE,    ///< True value.
      FALSE_VALUE    ///< False value.
    };
    
    /**
     * \brief A default constructor.
     */
    TriBool() : value(UNKNOWN_VALUE) {}
    
    /**
     * \brief A constructor.
     *
     * \param value for the initial true or false value.
     */
    TriBool(const bool initial_value) : value(initial_value ? TRUE_VALUE : FALSE_VALUE) {}

    /**
     * \brief Operator for conversion to a bool.
     *
     * \return bool returns true if the value is TRUE_VALUE, otherwise false.
     */
    operator bool() const
    {
      return value == TRUE_VALUE;
    }
    
    /**
     * \brief Operator for copy assignment.
     *
     * \param rhs for right hand side value.
     *
     * \return TriBool& self.
     */
    TriBool& operator=(const TriBool& other)
    {
      if (&other == this)
      {
        return *this;
      }

      value = other.value;

      return *this;
    }
    
    /**
     * \brief Operator for assignment.
     *
     * \param rhs for right hand side value.
     *
     * \return TriBool& self.
     */
    TriBool& operator=(const Values& rhs)
    {
      value = rhs;

      return *this;
    }
    
    /**
     * \brief Operator for assignment.
     *
     * \param rhs for right hand side value.
     *
     * \return TriBool& self.
     */
    TriBool& operator=(const bool& rhs)
    {
      value = (rhs ? TRUE_VALUE : FALSE_VALUE);

      return *this;
    }

    /**
     * \brief Operator for equal to comparison.
     *
     * \param rhs for right hand side value.
     *
     * \return bool indicating if the comparision was equal or not.
     */
    bool operator==(const TriBool& rhs) const
    {
      return value == rhs.value;
    }
    
    /**
     * \brief Operator for equal to comparison.
     *
     * \param rhs for right hand side value.
     *
     * \return bool indicating if the comparision was equal or not.
     */
    bool operator==(const Values& rhs) const
    {
      return value == rhs;
    }

    /**
     * \brief Operator for not equal to comparison.
     *
     * \param rhs for right hand side value.
     *
     * \return bool indicating if the comparision was not equal or not.
     */
    bool operator!=(const TriBool& rhs) const
    {
      return value != rhs.value;
    }
    
    /**
     * \brief Operator for not equal to comparison.
     *
     * \param rhs for right hand side value.
     *
     * \return bool indicating if the comparision was not equal or not.
     */
    bool operator!=(const Values& rhs) const
    {
      return value != rhs;
    }

    /**
     * \brief Operator for stream insertion.
     *
     * \param stream for the output stream.
     * \param rhs for the right hand side value, to insert into the output stream.
     *
     * \return std::ostream& for the output stream.
     */
    friend std::ostream& operator<<(std::ostream& stream, const TriBool& rhs)
    {
      return stream << rhs.toString();
    }

    /**
     * \brief A method for converting the tri bool to a text string.
     *
     * \return std::string containing the string representation of the value.
     */
    std::string toString() const
    {
      return (value == UNKNOWN_VALUE ? "unknown" : (value == TRUE_VALUE ? "true" : "false"));
    }

    /**
     * \brief The tri bool value.
     */
    Values value;
  };
  
  /**
   * \brief A struct for containing system information of the robot controller.
   */
  struct SystemInfo
  {
    /**
     * \brief The RobotWare version name.
     */
    std::string robot_ware_version;

    /**
     * \brief The system's name.
     */
    std::string system_name;
  };
  
  /**
   * \brief A struct for containing information about the RAPID tasks defined in the robot controller.
   */
  struct RAPIDTask
  {
    /**
     * \brief A constructor.
     *
     * \param name for the name of the task.
     * \param is_motion_task indicating if the task is a motion task or not.
     */
    RAPIDTask(std::string name, bool is_motion_task)
    :
    name(name),
    is_motion_task(is_motion_task)
    {}

    /**
     * \brief Flag indicating if the task is a motion task.
     */
    bool is_motion_task;

    /**
     * \brief The task's name.
     */
    std::string name;
  };
  
  /**
   * \brief A struct for containing static information (at least during runtime) about the robot controller.
   */
  struct StaticInfo
  {
    /**
     * \brief Information about the defined RAPID tasks.
     */
    std::vector<RAPIDTask> rapid_tasks;

    /**
     * \brief System information.
     */
    SystemInfo system_info;
  };
  
  /**
   * \brief A struct for containing runtime information about the robot controller.
   */
  struct RuntimeInfo
  {
    /**
     * \brief A default constructor.
     */
    RuntimeInfo() : rws_connected(false) {}

    /**
     * \brief Indicator for if the mode is auto or not or unknown.
     */
    TriBool auto_mode;
    
    /**
     * \brief Indicator for if the motors are on or not or unknown.
     */
    TriBool motor_on;
    
    /**
     * \brief Indicator for if RAPID is running or not or unknown.
     */
    TriBool rapid_running;
    
    /**
     * \brief Indicator for RWS is connected to the robot controller system.
     */
    bool rws_connected;
  };

  /**
   * \brief A constructor.
   *
   * \param ip_address specifying the robot controller's IP address.
   * \param port specifying the robot controller's RWS server's port number.
   */
  RWSInterface(const std::string ip_address, const unsigned short port = 80);
  
  /**
   * \brief A method for collecting runtime information of the robot controller.
   *
   * \return RuntimeInfo container for the runtime information.
   */
  RuntimeInfo collectRuntimeInfo();
  
  /**
   * \brief A method for collecting static information (at least during runtime) of the robot controller.
   *
   * \return StaticInfo container for the static information (at least during runtime).
   */
  StaticInfo collectStaticInfo();
  
  /**
   * \brief A method for retriving the value if an IO signal.
   *
   * \param iosignal for the name of the IO signal.
   *
   * \return std::string containing the IO signal's value (empty if not found).
   */
  std::string getIOSignal(const std::string iosignal);
  
  /**
   * \brief A method for retriving the current jointtarget values of a mechanical unit.
   * 
   * \param mechunit for the mechanical unit's name.
   * \param p_jointtarget for storing the retrived jointtarget data.
   *
   * \return bool indicating if the communication was successful or not. Note: NOT if it was parsed correctly.
   */
  bool getMechanicalUnitJointTarget(const std::string mechunit, JointTarget* p_jointtarget);
  
  /**
   * \brief A method for retriving the current robtarget values of a mechanical unit.
   * 
   * \param mechunit for the mechanical unit's name.
   * \param p_robtarget for storing the retrived robtarget data.
   *
   * \return bool indicating if the communication was successful or not. Note: NOT if it was parsed correctly.
   */
  bool getMechanicalUnitRobTarget(const std::string mechunit, RobTarget* p_robtarget);

  /**
   * \brief A method for retriving the data of an RAPID symbol (parsed into a struct representing the RAPID data).
   *
   * \param task for the name of the RAPID task containing the RAPID symbol.
   * \param symbol indicating the RAPID symbol resource (name and module).
   * \param p_data for storing the retrived RAPID symbol data.
   *
   * \return bool indicating if the communication was successful or not. Note: NOT if it was parsed correctly.
   */
  bool getRAPIDSymbolData(const std::string task,
                          const RWSClient::RAPIDSymbolResource symbol,
                          RAPIDSymbolDataAbstract* p_data);

  /**
   * \brief A method for retriving information about the RAPID tasks defined in the robot controller.
   *
   * \return std::vector<RAPIDTask> container for the RAPID tasks information.
   */
  std::vector<RAPIDTask> getRAPIDTasks();
  
  /**
   * \brief A method for retriving some system information from the robot controller.
   *
   * \return SystemInfo containing the system information.
   */
  SystemInfo getSystemInfo();
  
  /**
   * \brief A method for checking if the robot controller mode is in auto mode.
   *
   * \return TriBool indicating if the mode is auto or not or unknown.
   */
  TriBool isAutoMode();
  
  /**
   * \brief A method for checking if the motors are on.
   *
   * \return TriBool indicating if the motors are on or not or unknown.
   */
  TriBool isMotorOn();
  
  /**
   * \brief A method for checking if RAPID is running.
   *
   * \return TriBool indicating if RAPID is running or not or unknown.
   */
  TriBool isRAPIDRunning();

  /**
   * \brief A method for setting the value of an IO signal.
   *
   * \param iosignal for the name of the IO signal.
   * \param value for the IO signal's new value.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool setIOSignal(const std::string iosignal, const std::string value);
  
  /**
   * \brief A method for setting the data of an RAPID symbol.
   *
   * \param task for the name of the RAPID task containing the RAPID symbol.
   * \param symbol indicating the RAPID symbol resource (name and module).
   * \param data containing the RAPID symbol's new data.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool setRAPIDSymbolData(const std::string task,
                          const RWSClient::RAPIDSymbolResource symbol,
                          RAPIDSymbolDataAbstract& data);
  
  /**
   * \brief A method for starting RAPID execution in the robot controller.
   * 
   * \return bool indicating if the communication was successful or not.
   */
  bool startRAPIDExecution();
  
  /**
   * \brief A method for stopping RAPID execution in the robot controller.
   * 
   * \return bool indicating if the communication was successful or not.
   */
  bool stopRAPIDExecution();
  
  /**
   * \brief A method for reseting the RAPID program pointer in the robot controller.
   * 
   * \return bool indicating if the communication was successful or not.
   */
  bool resetRAPIDProgramPointer();

  /**
   * \brief A method for turning on the robot controller's motors.
   * 
   * \return bool indicating if the communication was successful or not.
   */
  bool setMotorsOn();
  
  /**
   * \brief A method for turning off the robot controller's motors.
   * 
   * \return bool indicating if the communication was successful or not.
   */
  bool setMotorsOff();

  /**
   * \brief A method for retriving a file from the robot controller.
   *
   * Note: Depending on the file, then the content can be in text or binary format.
   *
   * \param resource specifying the file's directory and name.
   * \param p_file_content for containing the retrived file content.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool getFile(const RWSClient::FileResource resource, std::string* p_file_content);

  /**
   * \brief A method for uploading a file to the robot controller.
   *
   * \param resource specifying the file's directory and name.
   * \param file_content for the file's content.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool uploadFile(const RWSClient::FileResource resource, const std::string file_content);

  /**
   * \brief A method for deleting a file from the robot controller.
   *
   * \param resource specifying the file's directory and name.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool deleteFile(const RWSClient::FileResource resource);
  
  /**
   * \brief A method for starting for a subscription.
   *
   * \param resources specifying the resources to subscribe to.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool startSubscription(const RWSClient::SubscriptionResources resources);
      
  /**
   * \brief A method for waiting for a subscription event.
   *
   * \param p_xml_document for storing the data received in the subscription event.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool waitForSubscriptionEvent(Poco::AutoPtr<Poco::XML::Document>* p_xml_document);
   
  /**
   * \brief A method for ending a active subscription.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool endSubscription();

  /**
   * \brief A method for registering a user as local.
   *
   * \param username specifying the user name.
   * \param application specifying the external application.
   * \param location specifying the location.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool registerLocalUser(std::string username = "Default User",
                         std::string application = "ExternalApplication",
                         std::string location = "ExternalLocation");
  
  /**
   * \brief A method for registering a user as remote.
   *
   * \param username specifying the user name.
   * \param application specifying the external application.
   * \param location specifying the location.
   *
   * \return bool indicating if the communication was successful or not.
   */
  bool registerRemoteUser(std::string username = "Default User",
                          std::string application = "ExternalApplication",
                          std::string location = "ExternalLocation");

  /**
   * \brief A method for retrieving the internal log as a text string.
   *
   * \param verbose indicating if the log text should be verbose or not.
   *
   * \return std::string containing the log text.
   */
  std::string getLogText(const bool verbose = false);

protected:
  /**
   * \brief A method for comparing a single text content (from a XML document node) with a specific string value.
   *
   * \param rws_result for containing a RWS communication result.
   * \param attribute for specifying the XML node's required attribute.
   * \param compare_string for specifying the comparison string.
   *
   * \return TriBool containing the result of the comparison.
   */
  TriBool compareSingleContent(const RWSClient::RWSResult& rws_result,
                               const XMLAttribute& attribute,
                               const std::string& compare_string);

  /**
   * \brief The RWS client used to communicate with the robot controller.
   */
  RWSClient rws_client_;
};

} // end namespace rws
} // end namespace abb

#endif