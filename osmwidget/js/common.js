// common.js
//
// This file contains some utility functions

// Check the error code and show the information to users
function checkError(message, resultList, divId, imgId)
{
  var errCode = resultList.ErrorCode;
  var msg = "";
  
  if (errCode) {
    msg = message + "<BR>" + "Failed Error: " + errCode + "<BR>";
    if(resultList.ErrorMessage != undefined)
      msg += "Error Message: " + resultList.ErrorMessage;
    showIMG(imgId,"no"); 
  } else {
    showIMG(imgId,"yes"); 
  }

  //print error message
  if(divId != null && divId != undefined)
    document.getElementById(divId).innerHTML = msg;
  console.info(msg);
  
  return errCode;
}

// Build the message by reading a iteratorable list in a recursive manner
function showIterableList(iterator)
{
  var msg = "";
  try
  {
    iterator.reset();
    var item;
    while (( item = iterator.getNext()) != undefined ){
       msg += showObject( item );
    }
  }
  catch(e)
  {
    alert('<showIterableList> ' + e);
  }
  return msg;
}

// Build the message by reading a JS object in a recursive manner
function showObject( obj )
{
  var txt = "";
  try { 
    if ( typeof obj != 'object' )
      return "" + obj + '<BR/>';
    else {
      for(var key in obj) {
        txt +=  key + ":";
        txt += showObject( obj[key] );
        txt += '<BR/>';
      }
      txt += '<BR/>';
    }
  } 
  catch (e) 
  {
    alert("showObject: " + e);
  }
  return txt;
}

// Show the image to indicate the test result
function showIMG(imgId, isOK) 
{
  if(imgId == null || imgId == undefined)
    return;
  
  if(isOK == "yes")
    document.getElementById(imgId).src = "pic/yes.png";
  else if(isOK == "no")
    document.getElementById(imgId).src = "pic/no.png";
  else
    document.getElementById(imgId).src = "pic/blank.png";
}

// Show elements in object by using 'alert'
function testObject(obj)
{
  var msg = "";
  for(var key in obj) {
    msg = msg + ":" + key + "=" + obj[key];
  }
  alert(msg);
}

// Test whether the input is numeric
function IsNumeric(sText)
{
  var ValidChars = "0123456789.";
  var IsNumber=true;
  var Char;
  
  for (i = 0; i < sText.length && IsNumber == true; i++) 
  { 
    Char = sText.charAt(i); 
    if (ValidChars.indexOf(Char) == -1) 
    {
       IsNumber = false;
    }
  }
  return IsNumber; 
}

