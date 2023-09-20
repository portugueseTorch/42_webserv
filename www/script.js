window.addEventListener('load', toDel);

function toDel() {
    const elems = document.getElementsByClassName("toDelete");

    for (i = 0; i < elems.length; i++) {
        elems[i].removeEventListener('click', deleteItem);
        elems[i].addEventListener('click', deleteItem);
    }
}

const deleteItem = (event) => {
    const name = event.target.parentNode.children[0].innerHTML;
    console.log(name);
    fetch('delete.py', {
        method: 'DELETE',
        body: 'name=' + name
    })
	.then(response => {
		if (!response.ok){
            throw new Error('Network response was not ok');
        }
		return fetch('/cgi-bin', {
            method: 'GET'
        });
	})
	.then(response => response.text())
    .then(data => {
		document.body.innerHTML = data;
		toDel();
	})
    .catch(error => {
        console.error('Error:', error);
    });
}

