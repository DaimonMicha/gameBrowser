
var kM = kM || function() {};

function checkProfile() {
    myGame.checkModules();
}

// @disable-check M307
var myGame = new kM({ container: 'accountPlugin'
                        , optionsClass: 'kmOptions'
                        , lastDuels: 5
                    });

window.setInterval(checkProfile, 500);
