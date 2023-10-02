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
	
	const reader = new FileReader();

	reader.onload = async function () {
		const binaryData = reader.result;

		const formData = new FormData();
		formData.append('name', file.name);
		formData.append('content', new Blob([binaryData]));

		// Convert ArrayBuffer to Uint8Array
		const uint8Array = new Uint8Array(binaryData);

		// Convert Uint8Array to hexadecimal string
		const hexString = Array.from(uint8Array).map(byte => byte.toString(16).padStart(2, '0')).join('');

		console.log(hexString);
		try {
			const response = await fetch('/images', {
				method: 'POST',
				body: formData
			});

			if (!response.ok) {
				throw new Error('Network response was not ok');
			}

			const data = await fetch('goodTimes.py', {
				method: 'GET'
			});
			
			const textData = await data.text();
			
			document.body.innerHTML = textData;
		} catch (error) {
			console.error('Error:', error);
		}
	};

	reader.readAsArrayBuffer(file);

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