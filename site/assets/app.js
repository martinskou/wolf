

function setup_filter() {
	const usp = new URLSearchParams(window.location.search);

    var e=document.getElementById('filter');
	e.value=usp.get('filter');	
	e.focus();
	e.select();
 
    e.addEventListener("keyup", function(event) {
        if (event.keyCode === 13) {
			event.preventDefault();
			var old_href=document.location.href.split("?")[0]
			var new_href=old_href+"?filter="+e.value;
			document.location.href=new_href;
        }
    });     

}
