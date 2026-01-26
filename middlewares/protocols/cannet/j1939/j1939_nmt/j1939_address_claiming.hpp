#pragma once

namespace ymd::j1939{

// https://github.com/Open-Agriculture/AgIsoStack-rs/blob/main/src/network_management/control_function.rs
enum class [[nodiscard]] AddressClaimingState {
    /// Address claiming is uninitialized
    None,
    /// State machine is waiting for the random delay time                       
    WaitForClaim,
    /// State machine is sending the request for address claim                
    SendRequestForClaim,
    /// State machine is waiting for the address claim contention period         
    WaitForRequestContentionPeriod,
    /// State machine is claiming the preferred address
    SendPreferredAddressClaim,
    /// State machine is contending the preferred address  
    ContendForPreferredAddress,
    /// State machine is claiming an address   
    SendArbitraryAddressClaim,
    /// An ECU requested address claim, inform the bus of our current address    
    SendReclaimAddressOnRequest,
    /// State machine could not claim an address
    UnableToClaim,
    /// Address claiming is complete and we have an address     
    AddressClaimingComplete,
}
}