R"(
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <title>OpenMacroRail</title>
    <meta name="viewport" content="width=device-width">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css"
        integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" crossorigin="anonymous">
    <style type="text/css">
        body {
            width: 100%;
            height: 100%;
            margin: 0px;
            background-color: transparent;
        }
        p {
            margin: 0px;
        }

        h1 {
            margin: 0px;
        }

        h2 {
            margin: 0px;
        }

        h3 {
            margin: 0px;
        }

        .gwd-div-juoq {
            left: 0px;
            top: 0px;
        }

        .gwd-jog-button {
            color: rgb(255, 255, 255);
            font-size: 50px;
        }

        .top-label {
            font-size: 1em;
        }

        /* This class is used on buttons that will be pressed repeatedly to prevent accidental zooms */
        .preventTapToZoom {
            touch-action: manipulation;
            -webkit-user-select: none;
            user-select: none;
        }

        .form-switch.form-switch-lg .form-check-input {
        height: 2rem;
        width: calc(3rem + 0.75rem);
        border-radius: 4rem;
        }

        .form-switch.form-switch-lg .form-check-label {
        padding-top: 0.5rem;
        padding-left: 1rem;
        }

        .form-switch.form-switch-xl .form-check-input {
        height: 2.5rem;
        width: calc(4rem + 0.75rem);
        border-radius: 5rem;
        }
        
        /* Limit max width for large screens to 720px */
        @media screen and (min-width: 992px){
            .container { max-width: 720px; }
        }
        @media screen and (min-width: 1200px){
            .container { max-width: 720px; }
        }
        @media screen and (min-width : 1400px) {
            .container { max-width: 720px; } 
        }
    </style>
</head>

<body class="htmlNoPages">
    <div class="container" id="MainDiv">
        <div class="row g-3 h-40" style="padding-top: 1em; padding-bottom: 1em;" id="TopSection">
            <div class="d-grid col-8 g-1" id="TopSettings">
                <p class="display-6" style="font-size: 1.3em;">Current position:</p>
                <h1 class="display-1" id="currentPosLabel"><span id="currentPosValue">0.000</span> <span id="unit-distance-currentPosValue">mm</span></h1>
                <p class="display-6 top-label">Jog increment:</p>
                <div class="btn-group" role="group" aria-label="Basic radio toggle button group">
                    <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrFast"
                        autocomplete="off" onchange='simpleAjaxCall("jogIncrFast")'>
                    <label class="btn btn-outline-secondary" for="buttonJogIncrFast"><span id="jogIncrFastValue">1.0</span> <span id="unit-distance-jogIncrFast">mm</span></label>

                    <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrMedium"
                        autocomplete="off" onchange='simpleAjaxCall("jogIncrMedium")'>
                    <label class="btn btn-outline-secondary" for="buttonJogIncrMedium"><span id="jogIncrMediumValue">0.5</span> <span id="unit-distance-jogIncrMedium">mm</span></label>

                    <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrSlow"
                        autocomplete="off" onchange='simpleAjaxCall("jogIncrSlow")' checked>
                    <label class="btn btn-outline-secondary" for="buttonJogIncrSlow"><span id="jogIncrSlowValue">0.1</span> <span id="unit-distance-jogIncrSlow">mm</span></label>
                </div>
                <label class="display-6 top-label form-label" for="numImagesForm">Number of images:</label>
                <input class="form-control " type="number" placeholder="eg. 200" value=200 step="1" min="0"
                    id="numImagesForm" onchange='formContentChanged(numImagesForm)'>
            </div>
            <div class="d-grid gap-2 col-4 mx-auto" id="TopButtons">
                <button type="button" class="btn btn-primary btn-floating btn-lg gwd-jog-button preventTapToZoom"
                    data-mdb-ripple-color="#ffffff" id="buttonJogFwd" onpointerup='simpleAjaxCall("jogFwd")'>↑</button>
                <button type="button" class="btn btn-primary btn-floating btn-lg gwd-jog-button preventTapToZoom"
                    data-mdb-ripple-color="#ffffff" id="buttonJogBwd" onpointerup='simpleAjaxCall("jogBwd")'>↓</button>
            </div>
        </div>
        <div style="padding-right: 0.7em;/*! padding-top: 1em; */ /*! padding-bottom: 1em; */" id="MidSection"
            class="row g-4 h-10">
            <div class="d-grid gap-2 col-5" id="MidLeft">
                <button type="button" class="btn btn-lg btn-primary  preventTapToZoom"
                    style="font-size: 1rem;" onpointerup='simpleAjaxCall("setStartPoint")'>Set startpoint</button>
                <button type="button" class="btn btn-lg btn-primary  preventTapToZoom"
                    style="font-size: 1rem;" onpointerup='simpleAjaxCall("setEndPoint")'>Set endpoint</button>
                <button type="button" class="btn btn-lg btn-secondary  preventTapToZoom"
                    style="font-size: 1rem;" onpointerup='simpleAjaxCall("testImage")'>Test image</button>
            </div>
            <div class="container d-flex flex-column col-7 gap-2" id="MidRight">
                <div class="row flex-grow-1">
                    <button type="button" class="btn btn-lg btn-success  preventTapToZoom"
                        onpointerup='simpleAjaxCall("start")'>Start shooting</button>
                </div>
                <div class="row h-33">
                    <button type="button" class="btn btn-lg btn-danger  preventTapToZoom"
                        style="font-size: 1rem;" onpointerup='simpleAjaxCall("abort")'>Abort shooting</button>
                </div>
            </div>
        </div>

        <div class="row g-1" style="padding-top: 1em; /*padding-bottom: 1em*/; --bs-gutter-y:0">
            <!-- <div class="col-8 ">Enable LED output</div> -->
            <div class="form-check form-switch form-switch-lg col-12">
                <input class="form-check-input" type="checkbox" role="switch" id="enableLEDSwitch" onchange='toggleLED()'>
                <label class="form-check-label" for="enableLEDSwitch">Enable LED output</label>
            </div>
            <label class="form-label" for="LEDBrightness" hidden>LED Brightness (Disabled)</label>
            <input class="form-range" type="range" id="LEDBrightness" min="0" max="100" , value="5" onchange='toggleLED()'
                disabled hidden>
        </div>

        <div class="row g-1 flex-grow-1" style="padding-top: 1em; padding-bottom: 1em;" id="BottomSection">
            
            <div class="accordion" id="accordionPanelsStayOpenExample">
                <!-- Stats section -->
                <div class="accordion-item">
                    <h6 class="accordion-header " id="panelsStayOpen-statsSectionHeader">
                        <button class="accordion-button" type="button" data-bs-toggle="collapse"
                            data-bs-target="#panelsStayOpen-statsSection" aria-expanded="true"
                            aria-controls="panelsStayOpen-statsSection">
                            Stats
                        </button>
                    </h6>
            
                    <div id="panelsStayOpen-statsSection" class="accordion-collapse collapse show"
                        aria-labelledby="panelsStayOpen-statsSectionHeader" style="padding: 1em">
                        <div class="row g-1" id="statsSection" >
                            <div class="col-8 ">Start point</div>
                            <div class="col-4 " id="startPointStatLabel"><span id="startPointValue">0.0</span> <span id="unit-distance-startPoint">mm</span></div>

                            <div class="col-8 ">End point</div>
                            <div class="col-4 " id="endPointStatLabel"><span id="endPointValue">0.0</span> <span id="unit-distance-endPoint">mm</span></div>

                            <div class="col-8 ">Total movement distance</div>
                            <div class="col-4 " id="distanceStatLabel"><span id="distanceValue">0.0</span> <span id="unit-distance-distance">mm</span></div>

                            <div class="col-8 ">Increment</div>
                            <div class="col-4 " id="incrementStatLabel"><span id="incrementValue">0.0</span> <span id="unit-distance-increment">mm</span></div>

                            <div class="col-8 ">Remaining pictures</div>
                            <div class="col-4 " id="remainingPicturesStatLabel"><span id="remainingPicturesValue">0</span></div>

                            <div class="col-8 ">Remaining shooting time</div>
                            <div class="col-4 " id="remainingShootTimeStatLabel"><span id="remainingShootingTimeValue">0.0</span> min</div>

                            <div class="col-8 ">Total shooting time</div>
                            <div class="col-4 " id="totalShootTimeStatLabel"><span id="totalShootingTimeValue">0.0</span> min</div>
                        </div>
                    </div>
                </div>


                <!-- Settings section -->
                <div class="accordion-item">
                    <h6 class="accordion-header " id="panelsStayOpen-settingsSectionHeader">
                        <button class="accordion-button" type="button" data-bs-toggle="collapse"
                            data-bs-target="#panelsStayOpen-settingsSection" aria-expanded="true"
                            aria-controls="panelsStayOpen-settingsSection">
                            Settings
                        </button>
                    </h6>

                    <div id="panelsStayOpen-settingsSection" class="accordion-collapse collapse show"
                        aria-labelledby="panelsStayOpen-settingsSectionHeader" style="padding: 1em">
                        
                        <div class="row g-1">
                            <div class="col-6"><label class="form-label" for="shootingSpeedForm">Shooting speed</label></div>
                            <div class="col-4 ">
                                <input class="form-control " type="number" placeholder="eg. 0.1" value=0.1 step="0.1"
                                min="0" max="10" id="shootingSpeedForm" onchange='formContentChanged(shootingSpeedForm)'>
                            </div>
                            <div class="col-2 "> <span id="unit-speed-shootingSpeed">mm/s</span></div>
                        </div>
                        
                        <div class="row g-1">
                            <div class="col-6"><label class="form-label" for="jogSpeedForm">Jog speed</label></div>
                            <div class="col-4 ">
                                <input class="form-control  " type="number" placeholder="eg. 1" value=1 step="0.1" min="0"
                                max="10" id="jogSpeedForm" onchange='formContentChanged(jogSpeedForm)'>
                            </div>
                            <div class="col-2 "> <span id="unit-speed-jogSpeed">mm/s</span></div>
                        </div>
                        
                        <div class="row g-1">
                            <div class="col-6"><label class="form-label" for="deshakeDelayForm">Deshake delay</label></div>
                            <div class="col-4 ">
                                <input class="form-control " type="number" placeholder="eg. 4" value=4 step="0.1" min="0"
                                max="60" id="deshakeDelayForm" onchange='formContentChanged(deshakeDelayForm)'>
                            </div>
                            <div class="col-2 "> s</div>
                        </div>
                        
                        <div class="row g-1">
                            <div class="col-6"><label class="form-label" for="shootDelayForm">Shoot delay</label></div>
                            <div class="col-4 ">
                                <input class="form-control " type="number" placeholder="eg. 0.5" value=0.5 step="0.1"
                                min="0" max="5" id="shootDelayForm" onchange='formContentChanged(shootDelayForm)'>
                            </div>
                            <div class="col-2 "> s</div>
                        </div>
                        
                        <div class="row g-1">
                            <div class="col-6"><label class="form-label" for="overshootDistanceForm">Overshoot distance</label></div>
                            <div class="col-4 ">
                                <input class="form-control " type="number" placeholder="eg. 0.5" value=0.5 step="0.1"
                                min="0" max="10" id="overshootDistanceForm" onchange='formContentChanged(overshootDistanceForm)'>
                            </div>
                            <div class="col-2 "> <span id="unit-distance-overshootDistance">mm</span></div>
                        </div>
                    </div>
                </div>
                
                
                <!-- Advanced settings section -->
                <div class="accordion-item">
                    <h6 class="accordion-header " id="panelsStayOpen-advSettingsSectionHeader">
                        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse"
                            data-bs-target="#panelsStayOpen-advSettingsSection" aria-expanded="true"
                            aria-controls="panelsStayOpen-advSettingsSection">
                            Advanced Settings
                        </button>
                    </h6>
                
                    <div id="panelsStayOpen-advSettingsSection" class="accordion-collapse collapse"
                        aria-labelledby="panelsStayOpen-advSettingsSectionHeader" style="padding: 1em">
                        
                        <div class="row g-1" style="padding-top: 0; padding-bottom: 0em; --bs-gutter-y:0" >
                            
                            <div class="row g-1" disabled hidden> <!-- TODO: Not implemented in API, may be a useful setting to expose so it's left here for the future -->    
                            <div class="col-6"><label class="form-label" for="motorStepSizeForm">Motor step size</label></div>
                                <div class="col-4 ">
                                    <input class="form-control " type="number" placeholder="eg. 200" value=200 step="1" min="0"
                                    max="1000" id="motorStepSizeForm" onchange='formContentChanged(motorStepSizeForm)'>
                                </div>
                                <div class="col-2 "> steps/rev</div>
                            </div>
                            
                            <div class="row g-1" disabled hidden> <!-- TODO: Not implemented in API, may be a useful setting to expose so it's left here for the future -->
                                <div class="col-6"><label class="form-label" for="gearRatioForm">Gear ratio</label></div>
                                <div class="col-4 ">
                                    <input class="form-control  " type="number" placeholder="eg. 1" value=1 step="0.1" min="0" max="10"
                                    id="gearRatioForm" onchange='formContentChanged(gearRatioForm)'>
                                </div>
                            </div>
                            
                            <div class="row g-1" disabled hidden> <!-- TODO: Not implemented in API, may be a useful setting to expose so it's left here for the future -->
                                <div class="col-8"><label class="form-check-label" for="toggleMotorDirectionSwitch">Invert directions</label></div>
                                <div class="form-check form-switch form-switch-lg col-4">
                                    <input class="form-check-input" type="checkbox" role="switch" id="toggleMotorDirectionSwitch"
                                    onchange='toggleMotorDirection()'>
                                </div>
                            </div>
                            
                            <div class="row g-1" >
                                <div class="col-8"><label class="form-check-label" for="enableImperialUnitsSwitch">Show imperial units</label></div>
                                <div class="form-check form-switch form-switch-lg col-4">
                                    <input class="form-check-input" type="checkbox" role="switch" id="enableImperialUnitsSwitch"
                                    onchange='toggleImperialUnits()'>
                                </div>
                            </div>
                            
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js"
        integrity="sha384-QJHtvGhmr9XOIpI6YVutG+2QOK9T+ZnN4kzFN1RtK3zEFEIsxhlmWl5/YESvpZ13"
        crossorigin="anonymous"></script>
    <script>
        window.onload = refreshSettingsAndStats;
        // Set up timers to poll the ESP32-server for changes to stats and settings. 
        // This keeps those entries in sync across devices accessing the server.
        let refreshStatsInterval = window.setInterval(refreshStats, 1000);
        let refreshSettingsInterval = window.setInterval(refreshSettings, 30000);

        var imperialUnitsEnabled = false;

        function insertStats(datastring) {
            console.log("Ajax result: " + datastring);
            let arr = datastring.split(",")
            for (let i = 0; i < arr.length; i++) {
                let infoPair = arr[i].split(":");
                if(infoPair.length == 1 ){
                    // Ignore entry if it contains no info. May typically happen at the end of the datastring
                    continue;
                }
                let id = infoPair[0] + "Value";
                var valueNode = document.getElementById(id);
                var value = infoPair[1];

                if (imperialUnitsEnabled) {
                    // All units are metric on the MCU (mm by default)
                    // Check if this node contains a unit that needs conversion
                    var unit = findMetricUnit(valueNode);
                    if (unit != null){
                        value = metricToImperialConverter(value, unit, maintainNumSignificantDigits=true);
                    }
                }
                
                valueNode.innerHTML = value;
            }
        }

        function insertSettings(datastring) {
            console.log("Ajax result: " + datastring);
            let arr = datastring.split(",")
            for (let i = 0; i < arr.length; i++) {
                let infoPair = arr[i].split(":");
                if (infoPair.length == 1) {
                    // Ignore entry if it contains no info. May typically happen at the end of the datastring
                    continue;
                }
                let id = infoPair[0] 
                let value = infoPair[1]
                switch (id) {
                    case "jogIncrement":
                        // The MCU will always operate with metric units, so this test does not need to be made imperial.
                        if (value === "1.0") { 
                            buttonJogIncrFast.checked = true;
                        } else if (value === "0.5") {
                            buttonJogIncrMedium.checked = true;
                        } else if (value === "0.1") {
                            buttonJogIncrSlow.checked = true;
                        }
                        break;
                    case "ledState":
                        enableLEDSwitch.checked = (value === "1");
                        updateLEDBrightnessSlider();
                        break;
                    case "ledBrightness":
                        LEDBrightness.value = value;
                        updateLEDBrightnessSlider();
                        break;
                    default:
                        id = id + "Form";
                        var valueNode = document.getElementById(id);

                        if (imperialUnitsEnabled && id != "numImagesForm") { // Special case for numImages as it doesn't have a unit, but findUnit will still find one. 
                            // All units are metric on the MCU (mm by default)
                            // Check if this node contains a unit that needs conversion
                            var unit = findMetricUnit(valueNode, recurse = true);
                            if (unit != null) {
                                value = metricToImperialConverter(value, unit);
                                // Note: we do not truncate numbers used as settings to avoid loosing presicion
                            }
                        }
                        document.getElementById(id).value = value;
                }
            }
        }

        function refreshStats() {
            $.ajax({
                "url": "refreshStats",
                "success": function (result) {
                    insertStats(result);
                }
            });
        }

        function refreshSettings() {
            $.ajax({
                "url": "refreshForms",
                "success": function (result) {
                    insertSettings(result);
                }
            });
        }

        function refreshUnitOfMeasurement(){
            //TODO: Load from cookie or server
            if(imperialUnitsEnabled){
                enableImperialUnitsSwitch.checked = true;
            }else{
                enableImperialUnitsSwitch.checked = false;
            }
        }

        function refreshSettingsAndStats() {
            refreshUnitOfMeasurement();
            refreshSettings();
            refreshStats();
            updateLEDBrightnessSlider()
        }

        function formContentChanged(formID) {
            if (formID.validity.valid === true) {
                var formValue = formID.value;
                if (imperialUnitsEnabled){
                    // Convert back to metric before sending update to MCU
                    let unit = findCurrentUnit(formID, recurse=true);
                    if (unit != null){
                        formValue = imperialToMetricConverter(formValue, unit); // Hardcoded to inches for now
                    }
                }
                makeAjaxCall(formID.id,
                    {
                        "num": formValue
                    });
                formID.style.color = "#000000"; //indicate valid data
                console.log(formID.id + " changed");
                refreshStats();
            } else {
                formID.style.color = "#ff0000"; //indicate invalid data
                console.log("Oopsie." + formID.id + " contains invalid data");
            }
        }

        function metricToImperialConverter(metricValue, metricUnit, maintainNumSignificantDigits=false){
            if (maintainNumSignificantDigits){
                // Store number of significant digits in the metric representation
                let digitSplit = metricValue.split(".")
                var numSignificantDigitsMetric = 0
                if (digitSplit.length == 2){
                    // The metric representation is a float
                    numSignificantDigitsMetric = digitSplit[1].length 
                }
            }
            var value = NaN
            
            // Perform conversion
            // All distance units are in millimeters rather than meters
            const inchesPerMillimeter = 0.0393700787;
            switch(metricUnit){
                case "mm":
                    value = metricValue * inchesPerMillimeter;
                    break;
                case "mm/s":
                    value = metricValue * inchesPerMillimeter;
                    break;
                default:
                    value = NaN; //Unknown unit. May already be converted to imperial
                    break;
            }

            if (maintainNumSignificantDigits){
                // Truncate value to equivalent number of decimals as the metric representation
                value = Number.parseFloat(value).toFixed(numSignificantDigitsMetric + 2) // Add two significant digits to account for scaling
            }

            return value
        }

        function imperialToMetricConverter(imperialValue, imperialUnit) {
            // All distance units are in millimeters rather than meters
            const inchesPerMillimeter = 0.0393700787;
            switch (imperialUnit) {
                case "in":
                    return imperialValue / inchesPerMillimeter;
                case "in/s":
                    return imperialValue / inchesPerMillimeter;
                default:
                    return NaN; //probably already converted to imperial?
            }
        }

        function findCurrentUnit(valueNode, recurse = false) {
            // Find sibling span to check if it contains a unit that needs conversion
            let unitNode = valueNode.parentNode.querySelector("[id^=unit]");
            if (unitNode == null && recurse){
                // Try same test one more level up (this is how forms are structured) 
                return findCurrentUnit(valueNode.parentNode, recurse=false);
            }
            var unit = null;
            if (unitNode != null) {
                unit = unitNode.innerHTML;
            }
            return unit;
        }

        function findMetricUnit(valueNode, recurse = false) {
            // Find sibling span to check if it contains a unit that needs conversion
            let unitNode = valueNode.parentNode.querySelector("[id^=unit]");
            if (unitNode == null && recurse) {
                // Try same test one more level up (this is how forms are structured) 
                return findMetricUnit(valueNode.parentNode, recurse = false);
            }
            var unit = null;
            if (unitNode != null) {
                if (unitNode.id.includes("distance")) {
                    unit = "mm";
                } else if (unitNode.id.includes("speed")) {
                    unit = "mm/s";
                } else if (unitNode.id.includes("accelleration")) {
                    unit = "mm/s<sup>2</sup>";
                }
            }
            return unit;
        }

        function convertDocumentToImperialUnits(){
            // Find all nodes that hods a unit that is not common between metric an imperial (time units are common and do not need to be updated)
            var unitNodes = document.querySelectorAll('[id^="unit"]'); // id starts with "unit"
            unitNodes.forEach(unitNode => {
                // Extract original unit
                var metricUnit =  unitNode.innerHTML;
                
                // Find first sibling-node whose id ends with Value  
                if(unitNode.parentNode.querySelector("[id$=Value]")){
                    // Has a sibling whose id ends with "Value"
                    var isForm = false;
                    var valueNode = unitNode.parentNode.querySelector("[id$=Value]"); // id ends with "Value"
                    var metricValue = valueNode.innerHTML;
                }else{
                    var isForm = true;
                    var valueNode = unitNode.parentNode.parentNode.querySelector("[id$=Form]");
                    var metricValue = valueNode.value;
                }
                
                var imperialValue = metricToImperialConverter(metricValue, metricUnit, maintainNumSignificantDigits=true);
                if (!isNaN(imperialValue)){
                    if (isForm){
                        valueNode.min = metricToImperialConverter(valueNode.min, metricUnit);
                        valueNode.max = metricToImperialConverter(valueNode.max, metricUnit);
                        valueNode.step = metricToImperialConverter(valueNode.step, metricUnit);
                        // recalculate imperial representation to avoid losing too much precision for settings
                        imperialValue = metricToImperialConverter(metricValue, metricUnit, maintainNumSignificantDigits = false); 
                        valueNode.value = imperialValue;
                    }else{
                        valueNode.innerHTML = imperialValue.toString();
                    }

                    if (unitNode.id.includes("distance")){
                        unitNode.innerHTML = "in"
                    } else if (unitNode.id.includes("speed")) {
                        unitNode.innerHTML = "in/s"
                    } else if (unitNode.id.includes("accelleration")) {
                        unitNode.innerHTML = "in/s<sup>2</sup>"
                    }
                }
            });
            imperialUnitsEnabled = true;
        }

        function convertDocumentToMetricUnits() {
            // Find all nodes that holds a unit that is not common between metric an imperial (time units are common and do not need to be updated)
            var unitNodes = document.querySelectorAll('[id^="unit"]'); 
            unitNodes.forEach(unitNode => {
                // reset min/max/step values for forms
                if (!unitNode.parentNode.querySelector("[id$=Value]")) {
                    // Extract original unit
                    var imperialUnit = unitNode.innerHTML;
                    var valueNode = unitNode.parentNode.parentNode.querySelector("[id$=Form]");
                    valueNode.min = imperialToMetricConverter(valueNode.min, imperialUnit);
                    valueNode.max = imperialToMetricConverter(valueNode.max, imperialUnit);
                    valueNode.step = imperialToMetricConverter(valueNode.step, imperialUnit);
                }
                // Update unit text
                if (unitNode.id.includes("distance")) {
                    unitNode.innerHTML = "mm"
                } else if (unitNode.id.includes("speed")) {
                    unitNode.innerHTML = "mm/s"
                } else if (unitNode.id.includes("accelleration")) {
                    unitNode.innerHTML = "mm/s<sup>2</sup>"
                }
            });

            // Special case for buttons
            jogIncrFastValue.innerHTML = "1.0";
            jogIncrMediumValue.innerHTML = "0.5";
            jogIncrSlowValue.innerHTML = "0.1";
            
            imperialUnitsEnabled = false;
            
            // Update values by requesting original values from MCU
            // This prevents loss of presicion by converting back from imperial to metric
            refreshSettingsAndStats();
        }

        function toggleImperialUnits(){
            if(imperialUnitsEnabled){
                convertDocumentToMetricUnits();
            }else{
                convertDocumentToImperialUnits();
            }
        }

        function toggleAdvSettingsSwitch(){
            if (showAdvSettingsSwitch.checked) {
                toggleAdvSettingsLabel.innerHTML = "Hide Advanced Settings"
                advancedSettingsSection.hidden = false;
            } else {
                toggleAdvSettingsLabel.innerHTML = "Show Advanced Settings"
                advancedSettingsSection.hidden = true;
            }
        }

        function updateLEDBrightnessSlider() {
            if (enableLEDSwitch.checked) {
                LEDBrightness.disabled = false;
                LEDBrightness.labels[0].textContent = "LED Brightness";
                LEDBrightness.hidden = false;
                LEDBrightness.labels[0].hidden = false;
            } else {
                LEDBrightness.disabled = true;
                LEDBrightness.labels[0].textContent = 'LED Brightness (Disabled)';
                LEDBrightness.hidden = true;
                LEDBrightness.labels[0].hidden = true;
            }
        }

        function toggleLED() {
            updateLEDBrightnessSlider();
            makeAjaxCall("ledSwitch",
                {
                    "state": enableLEDSwitch.checked,
                    "brightnessPercent": LEDBrightness.value
                });
        }

        function makeAjaxCall(url, data) {
            $.ajax({
                "url": url,
                "data": data,
                "success": function (result) {
                    console.log("Ajax result: " + result);
                }
            });
        }
        function simpleAjaxCall(url) {
            $.ajax({ "url": url });
            refreshStats();
        }
    </script>
</body>

</html>
)"
