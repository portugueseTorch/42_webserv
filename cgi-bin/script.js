window.addEventListener('load', toDel);

function toDel() {
    const elems = document.getElementsByClassName("toDelete");

    for (i = 0; i < elems.length; i++) {
        elems[i].removeEventListener('click', deleteItem);
        elems[i].addEventListener('click', function (){
			deleteItem(this);
		});
    }
}

const deleteItem = (element) => {
	const params = new URLSearchParams();

	['name', 'age', 'location', 'comment'].forEach(attribute => {
		const value = element.parentNode.querySelector(`.display-${attribute}`).textContent;
		params.append(attribute, value);
	});

    fetch('delete.py', {
        method: 'DELETE',
        body: params.toString()
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

