// window.addEventListener('load', selectFiles);

// function selectFiles() {
// 	const selectedFile = document.getElementById("file-content");
// 	console.log(selectedFile);
// 	selectedFile.addEventListener("change", handleFiles, false);
// }


// function handleFiles() {
// 	const fileList = this.files;
// 	console.log(fileList);
// }

const uploadFile = ((event) => {
	event.preventDefault();
	
	const file = document.getElementById('file-content').files[0];

	console.log(file);

	if (!file.type.startsWith('image/'))
		return ;
	
	// const reader = new FileReader();
	const readTextFile = async (file, finalText) => {
		const text = await file.text();
		finalText = text;
	}
	const name = file.name;

	let finalText = "";
	readTextFile(file, finalText);

	const params = [];

	params.push(`name=${name}`);
	params.push(`content=${finalText}`);
	// ['content'].forEach((attr) => {
	// 	const value = document.querySelector(`#file-${attr}`).value;
	// 	params.push(`${attr}=${value}`);
	// })

	const body = params.join('&');
	
	fetch('/images', {
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