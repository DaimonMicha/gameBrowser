
var km_profile = new kmProfile();

function checkProfile(){
    km_profile.checkProfile();
}

var boxes = $$('input.kmChecker');
boxes.each(function(box) {
    if(account.isActive(box.id)) box.setAttribute('checked', 'checked');
    box.addEvent('click', function() { account.toggle(box.id, box.checked); });
});

window.setInterval(checkProfile, 500);
