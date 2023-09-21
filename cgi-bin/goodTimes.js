const uploadFile = ((event) => {
	event.preventDefault();
	
	const params = [];

	['name', 'content'].forEach((attr) => {
		const value = document.querySelector(`#file-${attr}`).value;
		params.push(`${attr}=${value}`);
	})

	const body = params.join('&');
	
	fetch('/resources', {
		method: 'POST',
		body: body
	})
	.then(response => {
		if (!response.ok) {
			throw new Error('Network response was not ok');
		}
		return fetch('goodTimes.py', {
            method: 'GET'
        });
	})
	.then(response => response.text())
	.then(data => {
		document.body.innerHTML = data;
	})
	.catch(error => {
		console.error('Error:', error);
	});
});

const deleteFile = ((element) => {
	const pathAfterComma = element.textContent.split(', ')[1];
	console.log(pathAfterComma);

	fetch('/resources', {
		method: 'DELETE',
		body: pathAfterComma
	})
	.then(response => {
		if (!response.ok) {
			throw new Error('Network response was not ok');
		}
		return fetch('goodTimes.py', {
            method: 'GET'
        });
	})
	.then(response => response.text())
	.then(data => {
		document.body.innerHTML = data;
	})
	.catch(error => {
		console.error('Error:', error);
	});
})