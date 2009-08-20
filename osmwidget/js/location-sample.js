// location-sample.js
// 
// In this sample Location will be listed, traced, and calculated. 
// Also, Also, async operation will be canceled.
//

//SAPI Error Codes
// 0  - Success
// 1000 - Incorrect argument
// 1001 - UnknownArgumentName
// 1002 - BadArgumentType
// 1003 - MissingArgument
// 1004 - ServiceNotSupported
// 1005 - ServiceInUse
// 1006 - ServiceNotReady
// 1007 - NoMemory
// 1008 - HardwareNotAvailable
// 1009 - ServerBusy
// 1010 - EntryExists
// 1011 - AccessDenied
// 1012 - NotFound
// 1013 - UnknownFormat
// 1014 - GeneralError
// 1015 - CancelSuccess
// 1016 - ServiceTimedOut
// 1017 - File already exist

// Declare the service object 
var so;

// imgid for callback1 function
var imgid_callback1;

// imgid for callback2 function
var imgid_callback2;

// id of the div used to display information
const DIV_ID = 'locationsample';

// Called from onload()        
function setup()
{
  try {
    so = device.getServiceObject("Service.Location", "ILocation");
    //console.info("setup: so: %s", so);
  }
  catch (e) {   
      alert('<setup> ' +e);
  }
}

// Get Location
function getLocation(imgId) {
  
  // This specifies update option used while retrieving location estimation. 
  var updateoptions = new Object();
  // Setting PartialUpdates to 'FALSE' ensures that user get atleast 
  // BasicLocationInformation (Longitude, Lattitude, and Altitude.)
  updateoptions.PartialUpdates = false;
  
  var criteria = new Object();
  criteria.LocationInformationClass = "GenericLocationInfo";
  criteria.Updateoptions = updateoptions;
  
  try {
    var result = so.ILocation.GetLocation(criteria);
    if(!checkError("ILocation::getLocation",result,DIV_ID,imgId)) {
      document.getElementById(DIV_ID).innerHTML = showObject(result.ReturnValue);
    }    
  }
  catch (e) {
    showIMG(imgId,"no");
    alert ("getLocation: " + e);
  }
}

// Get Location Async
function getLocationAsync() {
  
  // This specifies update option used while retrieving location estimation. 
  var updateoptions = new Object();
  // Setting PartialUpdates to 'FALSE' ensures that user get atleast 
  // BasicLocationInformation (Longitude, Lattitude, and Altitude.)
  updateoptions.PartialUpdates = false;
  
  var criteria = new Object();
  criteria.LocationInformationClass = "GenericLocationInfo";
  criteria.Updateoptions = updateoptions;
  
  try {     
    var result = so.ILocation.GetLocation(criteria, callback1);
    if(!checkError("ILocation::getLocationAsync",result,DIV_ID,imgId)) {
    }    
  }
  catch (e) {
  }
}

// Trace Location
function traceLocation(imgId) {
  
  // This specifies update option used while retrieving location estimation. 
  var updateoptions = new Object();
  // Setting PartialUpdates to 'FALSE' ensures that user get atleast 
  // BasicLocationInformation (Longitude, Lattitude, and Altitude.)
  updateoptions.PartialUpdates = false;
  
  var criteria = new Object();
  criteria.LocationInformationClass = "GenericLocationInfo";
  criteria.Updateoptions = updateoptions;
  
  // Set image id for callback2 function
  imgid_callback2 = imgId;
  
  try {
    var result = so.ILocation.Trace(criteria, callback2);
    if(!checkError("ILocation::traceLocation",result,DIV_ID,imgId)) {
      showIMG(imgId,"");
    }    
  }
  catch (e) {
    showIMG(imgId,"no");
    alert ("traceLocation: " + e);
  }
}

// Cancel Notification
function cancelNotification(imgId) {
  
  // "TraceCancel"
  // "GetLocCancel"
  var msg = "Please enter the type ['GetLocCancel' or 'TraceCancel']";
  var cancelRequestType = prompt(msg, "TraceCancel");
  
  if(cancelRequestType == "" || cancelRequestType == null)
    return;

  var criteria = new Object();
  criteria.CancelRequestType = cancelRequestType;

  try {
    var result = so.ILocation.CancelNotification(criteria);
    checkError("ILocation::cancelNotification",result,DIV_ID,imgId);   
  }
  catch (e) {
    showIMG(imgId,"no");
    alert ("cancelNotification: " + e);
  }
}

// Calculate Location
function calculateLocation(imgId) {
  
  // "FindDistance"
  // "FindBearingTo"
  // "MoveCoordinates"
  var msg = "enter ['FindDistance' or 'FindBearingTo' or 'MoveCoordinates']";
  var mathRequest = prompt(msg, "FindDistance");
  
  if(mathRequest == "" || mathRequest == null)
    return;

  var distanceSrc = new Object();
  distanceSrc.Latitude = 40.69;
  distanceSrc.Longitude = 74.04; 
  distanceSrc.Altitude = 59;

  var distanceDest = new Object();
  distanceDest.Latitude = 42.48;
  distanceDest.Longitude = 71.19;
  distanceDest.Altitude = 2;

  var criteria = new Object();
  criteria.MathRequest = mathRequest;
  criteria.DistanceParamSource = distanceSrc;
  criteria.DistanceParamDestination = distanceDest;
  criteria.MoveByThisDistance = 10.0;
  criteria.MoveByThisBearing = 10.0;

  try {
    var result = so.ILocation.Calculate(criteria);
    if(!checkError("ILocation::Calculate",result,DIV_ID,imgId)) {
      document.getElementById(DIV_ID).innerHTML = showObject(result.ReturnValue);
    }
  }
  catch (e) {
    showIMG(imgId,"no");
    alert ("calculateLocation: " + e);
  }
}

// This is the asynchronous callback handler 
function callback1(transId, eventCode, result)
{
  console.info("getLocationAsync: transId: %d  eventCode: %d result.ErrorCode: %d", transId, eventCode, result.ErrorCode);
  if(!checkError("ILocation::getLocationAsync",result,DIV_ID,imgid_callback1)) {
    document.getElementById(DIV_ID).innerHTML = showObject(result.ReturnValue);
  }
}

// This is the asynchronous callback handler 
function callback2(transId, eventCode, result)
{
  console.info("getLocationAsync: transId: %d  eventCode: %d result.ErrorCode: %d", transId, eventCode, result.ErrorCode);
  if(!checkError("ILocation::traceLocation",result,DIV_ID,imgid_callback2)) {
    document.getElementById(DIV_ID).innerHTML = showObject(result.ReturnValue);
  }
}