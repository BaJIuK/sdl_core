/*
 * Copyright (c) 2013, Ford Motor Company All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  · Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *  · Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *  · Neither the name of the Ford Motor Company nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
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
/*
 * Reference implementation of UI component.
 *
 * Interface to get or set some essential information sent from SDLCore. UI is
 * responsible for the functionality provided by the application: display
 * graphics and multimedia components, is responsible for the transfer of
 * managed manipulations, generated by the user to the server.
 *
 */

FFW.UI = FFW.RPCObserver.create( {

    /**
     * If true then UI is present and ready to communicate with SDL.
     *
     * @type {Boolean}
     */
    isReady: false,

    /*
     * access to basic RPC functionality
     */
    client: FFW.RPCClient.create( {
        componentName: "UI"
    } ),

    // temp var for debug
    appId: 1,

    onVRChoiseSubscribeRequestId: -1,
    onVRChoiseUnsubscribeRequestId: -1,

    // const
    onVRChoiseNotification: "VR.OnChoise",

    /*
     * ids for requests AudioPassThru
     */
    performAudioPassThruRequestId: -1,
    endAudioPassThruRequestId: -1,

    /*
     * connect to RPC bus
     */
    connect: function() {
        this.client.connect( this, 400 );
    },

    /*
     * disconnect from RPC bus
     */
    disconnect: function() {
        this.client.disconnect();
    },

    /*
     * Client is registered - we can send request starting from this point of
     * time
     */
    onRPCRegistered: function() {
        Em.Logger.log( "FFW.UI.onRPCRegistered" );
        this._super();

        // subscribe to notifications
        this.onVRChoiseSubscribeRequestId = this.client.subscribeToNotification( this.onVRChoiseNotification );
    },

    /*
     * Client is unregistered - no more requests
     */
    onRPCUnregistered: function() {
        Em.Logger.log( "FFW.UI.onRPCUnregistered" );
        this._super();

        // unsubscribe from notifications
        this.onVRChoiseUnsubscribeRequestId = this.client.unsubscribeFromNotification( this.onVRChoiseNotification );
    },

    /*
     * Client disconnected.
     */
    onRPCDisconnected: function() {

    },

    /*
     * when result is received from RPC component this function is called It is
     * the propriate place to check results of request execution Please use
     * previously store reuqestID to determine to which request repsonse belongs
     * to
     */
    onRPCResult: function( response ) {
        Em.Logger.log( "FFW.UI.onRPCResult" );
        this._super();
    },

    /*
     * handle RPC erros here
     */
    onRPCError: function( error ) {
        Em.Logger.log( "FFW.UI.onRPCError" );
        this._super();
    },

    /*
     * handle RPC notifications here
     */
    onRPCNotification: function( notification ) {
        Em.Logger.log( "FFW.UI.onRPCNotification" );
        this._super();

        if( notification.method == this.onVRChoiseNotification ){
            this.interactionResponse( SDL.SDLModel.resultCode["SUCCESS"], notification.params.choiceId );
        }
    },

    /*
     * handle RPC requests here
     */
    onRPCRequest: function( request ) {
        Em.Logger.log( "FFW.UI.onRPCRequest" );
        this._super();

        var resultCode = null;

        switch( request.method ){
            case "UI.Show": {

                // Close TBT
                SDL.TurnByTurnView.deactivate();

                SDL.SDLController.getApplicationModel( request.params.appId ).onSDLUIShow( request.params );

                this.sendResponse(request.id, 0, 'Show');

                break;
            }
            case "UI.Alert": {

                SDL.SDLModel.onUIAlert( request.params, request.id );

                break;
            }
            case "UI.SetGlobalProperties": {

                SDL.SDLModel.setProperties( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.ResetGlobalProperties": {

                // reset all requested properties
                SDL.SDLModel.resetProperties( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.AddCommand": {

                SDL.SDLController.getApplicationModel( request.params.appId ).addCommand( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.DeleteCommand": {

                SDL.SDLController.getApplicationModel( request.params.appId ).deleteCommand( request.params.cmdId );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.AddSubMenu": {

                SDL.SDLController.getApplicationModel( request.params.appId ).addSubMenu( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.DeleteSubMenu": {

                var resultCode = SDL.SDLController.getApplicationModel( request.params.appId ).deleteSubMenu( request.params.menuId );

                this.sendUIResult( resultCode, request.id, request.method );

                break;
            }
            case "UI.CreateInteractionChoiceSet": {

                SDL.SDLController.getApplicationModel( request.params.appId ).onCreateInteraction( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.DeleteInteractionChoiceSet": {

                SDL.SDLController.getApplicationModel( request.params.appId ).onDeleteInteraction( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.PerformInteraction": {

                SDL.SDLModel.uiPerformInteraction( request.params, request.id );

                break;
            }
            case "UI.SetMediaClockTimer": {

                var resultCode = SDL.SDLController.getApplicationModel( request.params.appId ).sdlSetMediaClockTimer( request.params );

                this.sendUIResult( resultCode, request.id, request.method );

                break;
            }
            case "UI.OnAppActivated": {

                break;
            }
            case "UI.Slider": {

                SDL.SDLModel.uiSlider( request );

                break;
            }
            case "UI.ScrollableMessage": {

                SDL.SDLModel.onSDLScrolableMessage( request.params, request.id );

                break;
            }
            case "UI.ChangeRegistration": {

                SDL.SDLModel.changeRegistrationUI( request.params.hmiDisplayLanguage );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.SetAppIcon": {

                SDL.SDLModel.onSDLSetAppIcon( request.params, request.id, request.method );

                break;
            }
            case "UI.PerformAudioPassThru": {

                this.performAudioPassThruRequestId = request.id;

                SDL.SDLModel.UIPerformAudioPassThru( request.params );

                break;
            }
            case "UI.EndAudioPassThru": {

                this.endAudioPassThruRequestId = request.id;

                SDL.SDLModel.UIEndAudioPassThru();

                break;
            }
            case "UI.GetSupportedLanguages": {

                var JSONMessage = {
                    "jsonrpc": "2.0",
                    "id": request.id,
                    "result": {
                        "resultCode": SDL.SDLModel.resultCode["SUCCESS"], // type (enum) from SDL
                                                    // protocol
                        "method": "UI.GetSupportedLanguagesResponse",
                        "languages": SDL.SDLModel.sdlLanguagesList
                    }
                };
                this.client.send( JSONMessage );

                break;
            }
            case "UI.GetLanguage": {

                var JSONMessage = {
                    "jsonrpc": "2.0",
                    "id": request.id,
                    "result": {
                        "resultCode": SDL.SDLModel.resultCode["SUCCESS"], // type (enum) from SDL
                                                    // protocol
                        "method": "UI.GetLanguageResponse",
                        "hmiDisplayLanguage": SDL.SDLModel.hmiUILanguage
                    }
                };
                this.client.send( JSONMessage );

                break;
            }
            case "UI.ShowConstantTBT": {

                SDL.SDLModel.tbtActivate( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.UpdateTurnList": {

                SDL.SDLModel.tbtTurnListUpdate( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.AlertManeuver": {

                SDL.SDLModel.onUIAlertManeuver( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.DialNumber": {

                SDL.SDLModel.dialNumber( request.params );

                this.sendUIResult( SDL.SDLModel.resultCode["SUCCESS"], request.id, request.method );

                break;
            }
            case "UI.GetCapabilities": {
                // send repsonse
                var JSONMessage = {
                    "jsonrpc": "2.0",
                    "id": request.id,
                    "result": {
                        "displayCapabilities": {
                            "displayType": "GEN2_8_DMA",
                            "textFields":
                                [
                                    {
                                        "fieldName": "mainField1"
                                    },
                                    {
                                        "fieldName": "mainField2"
                                    },
                                    {
                                        "fieldName": "statusBar"
                                    },
                                    {
                                        "fieldName": "mediaClock"
                                    },
                                    {
                                        "fieldName": "mediaTrack"
                                    },
                                    {
                                        "fieldName": "alertText1"
                                    },
                                    {
                                        "fieldName": "alertText2"
                                    }
                                ],
                            "mediaClockFormats":
                                [
                                    "CLOCK1",
                                    "CLOCK2",
                                    "CLOCKTEXT1",
                                    "CLOCKTEXT2",
                                    "CLOCKTEXT3"
                                ],
                            "graphicSupported": true
                        },
                        "hmiZoneCapabilities":
                            [
                                "FRONT",
                                "BACK"
                            ],
                        "softButtonCapabilities":
                            [
                                {
                                    "shortPressAvailable": true,
                                    "longPressAvailable": true,
                                    "upDownAvailable": true,
                                    "imageSupported": true
                                }
                            ],
                        "code": 0,
                        "method": "UI.GetCapabilities"
                    }
                };

                this.client.send( JSONMessage );

                break;
            }

            case "UI.IsReady": {

                // send repsonse
                var JSONMessage = {
                    "jsonrpc": "2.0",
                    "id": request.id,
                    "result": {
                        "available": this.get('isReady'),
                        "code": 0,
                        "method" : "UI.IsReady"
                    }
                };

                this.client.send( JSONMessage );

                break;
            }

            default: {
                // statements_def
                break;
            }
        }
    },

    /**
     * Updated version of response
     * with no additional params
     *
     * @param {Number} responseId
     * @param {Number} codeId
     * @param {String} responseMethod
     */
    sendResponse: function( responseId, codeId, responseMethod ) {
        var JSONMessage = {
                "jsonrpc": "2.0",
                "id": responseId,
                "result": {
                    "code": codeId,
                    "method" : "UI." + responseMethod
            }
        };

        this.client.send( JSONMessage );
    },

    /*
     * send response from onRPCRequest
     */
    sendUIResult: function( resultCode, id, method ) {

        Em.Logger.log( "FFW.UI." + method + "Response" );

        if( resultCode ){

            // send repsonse
            var JSONMessage = {
                "jsonrpc": "2.0",
                "id": id,
                "result": {
                    "code": resultCode, // type (enum) from SDL protocol
                    "method": method
                }
            };
            this.client.send( JSONMessage );
        }
    },

    /**
     * send response from onRPCRequest
     */
    alertResponse: function( resultCode, id ) {

        Em.Logger.log( "FFW.UI.AlertResponse" );

        if( resultCode ){

            // send repsonse
            var JSONMessage = {
                "jsonrpc": "2.0",
                "id": id,
                "result": {
                    "code": resultCode, // type (enum) from SDL protocol
                    "method": 'UI.Alert'
                }
            };
            this.client.send( JSONMessage );
        }
    },

    sendSliderResult: function( resultCode, sliderRequestId, sliderPosition ) {
        var JSONMessage = {
            "jsonrpc": "2.0",
            "id": sliderRequestId,
            "result": {
                "code": resultCode, // type (enum) from SDL protocol
                "method": "UI.SliderResponse"
            }
        };

        if( sliderPosition ){
            JSONMessage.result.sliderPosition = sliderPosition;
        }

        this.client.send( JSONMessage );
    },

    /*
     * send notification when command was triggered
     */
    onCommand: function( commandId, appId ) {
        Em.Logger.log( "FFW.UI.onCommand" );

        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnCommand",
            "params": {
                "commandId": commandId,
                "appId": appId
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * send notification when command was triggered
     */
    onCommandSoftButton: function( softButtonID, appId ) {
        Em.Logger.log( "FFW.UI.onCommand" );

        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnCommand",
            "params": {
                "commandId": softButtonID,
                "appId": appId
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * send notification when command was triggered
     */
    interactionResponse: function( resultCode, performInteractionRequestId, commandId ) {
        Em.Logger.log( "FFW.UI.PerformInteractionResponse" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "id": performInteractionRequestId,
            "result": {
                "code": resultCode,
                "method": "UI.PerformInteractionResponse"
            }
        };

        if( commandId ){
            JSONMessage.result.choiceID = commandId;
        }

        this.client.send( JSONMessage );
    },

    /*
     * send notification when DriverDistraction PopUp is visible
     */
    onDriverDistraction: function( driverDistractionState ) {
        Em.Logger.log( "FFW.UI.DriverDistraction" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnDriverDistraction",
            "params": {
                "state": driverDistractionState,
                "appId": 0
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * Notifies if system context is changed
     */
    OnSystemContext: function( systemContextValue ) {
        Em.Logger.log( "FFW.UI.OnSystemContext" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnSystemContext",
            "params": {
                "systemContext": systemContextValue
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * Notifies if application was activated
     */
    OnAppActivated: function( appName ) {
        Em.Logger.log( "FFW.UI.OnAppActivated" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnAppActivated",
            "params": {
                "appName": appName
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * Notifies if device was choosed
     */
    OnDeviceChosen: function( deviceName ) {
        Em.Logger.log( "FFW.UI.OnDeviceChosen" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnDeviceChosen",
            "params": {
                "deviceName": deviceName
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * Notifies if sdl UI components language was changed
     */
    OnLanguageChange: function( lang ) {
        Em.Logger.log( "FFW.UI.OnLanguageChange" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnLanguageChange",
            "params": {
                "hmiDisplayLanguage": lang
            }
        };
        this.client.send( JSONMessage );
    },

    /*
     * Notifies if TBTClientState was activated
     */
    onTBTClientState: function( state, appId ) {
        Em.Logger.log( "FFW.UI.OnTBTClientState" );

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "UI.OnTBTClientState",
            "params": {
                "state": state,
                "appId": appId
            }
        };
        this.client.send( JSONMessage );
    }
} )
