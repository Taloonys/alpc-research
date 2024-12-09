/**
 * @brief Server code to use ALPC
 */

#include <array>
#include <cassert>      // dbg
#include <iostream>
#include <exception>
#include <windows.h>
#include <winternl.h>
#include "ntalpcapi.h"


//
// PORT MESSAGE contains : header + data
// Any message to ALPC could include ATTRIBUTES. Attributes that u r gonna use 
//   - should be validated
// You could reserve buffer for all attributes but don't use them for concrete messages
//


namespace details {

    static constexpr auto kMaxMsgLen = 0x500;

    static const auto kPortName = L"alpc-test-server";

} // namespace details


struct PortInfo {
    HANDLE                  handle;
    HANDLE                  connection;
    OBJECT_ATTRIBUTES       obj_attributes;
    ALPC_PORT_ATTRIBUTES    srv_port_attributes;
    UNICODE_STRING          name;

    bool Close() const noexcept {
        CloseHandle(handle) && CloseHandle(connection);
    }
};


int main() try 
{
    //
    // Port config
    //

    PortInfo port = {};

    ::RtlInitUnicodeString(&port.name, details::kPortName);

    InitializeObjectAttributes(
        &port.obj_attributes, 
        &port.name, 
        0, 0, 0);

    port.srv_port_attributes.MaxMessageLength = details::kMaxMsgLen;

    //
    // Run server
    //

    auto port_status = ::NtAlpcCreatePort(
        &port.handle,
        &port.obj_attributes,
        &port.srv_port_attributes);
    assert(NT_SUCCESS(port_status));

    std::cout << ">>> [Server] Created to port\n";

    //
    // Init message
    //

    PORT_MESSAGE pm_receive = {};

    auto leng = sizeof(pm_receive);
    auto init_msg_status = ::NtAlpcSendWaitReceivePort(
        &port.handle, 
        0, 
        nullptr, 
        nullptr, 
        &pm_receive, 
        &leng, 
        nullptr, 
        nullptr);
    assert(NT_SUCCESS(init_msg_status));

    std::cout << ">>> [Server] Got init_msg length = " << leng << std::endl;

    //
    // Connect
    //

    PORT_MESSAGE pm_request = {};

    RtlSecureZeroMemory(&pm_request, sizeof(pm_request));
    pm_request.MessageId            = pm_receive.MessageId;
    pm_request.u1.s1.DataLength     = 0x0;
    pm_request.u1.s1.TotalLength    = pm_request.u1.s1.DataLength + sizeof(PORT_MESSAGE);

    auto connect_status = ::NtAlpcAcceptConnectPort(
        &port.connection, 
        &port.handle, 
        0, 
        nullptr, 
        nullptr, 
        nullptr, 
        &pm_request, 
        nullptr, 
        TRUE);
    assert(NT_SUCCESS(connect_status));

    std::cout << ">>> [Server] Accepted connection to port\n";

    //
    // Wait for HANDLE_ATTRIBUTE
    //

    std::array<std::byte, 1024> buffer;
    auto inbound_msg_status = NtAlpcSendWaitReceivePort(
        port.handle, 
        0, 
        nullptr, 
        nullptr, 
        (PPORT_MESSAGE)&buffer, 
        &leng, 
        nullptr, 
        nullptr);
    assert(NT_SUCCESS(inbound_msg_status));

    std::cout << ">>> [Server] got HANDLE_ATTR, msg len = " << leng << std::endl;

    //
    // Close handles
    // TODO: class + RAII
    //

    port.Close();
} 
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}