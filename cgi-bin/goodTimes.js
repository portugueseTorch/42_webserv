function clearOutput() {
	const fileContent = document.getElementById("file-content-text");
	fileContent.innerHTML = "No file selected";
	console.log('aqui')
}

function resetBorders() {
    const uploadedFiles = document.querySelectorAll('.uploaded-file');
    uploadedFiles.forEach(file => {
        file.style.border = '1px solid #ccc';
    });
}

const deleteFile = ((fileName, parentElement) => {

	resetBorders();

	fetch('goodTimes.py', {
		method: 'DELETE',
		body: 'webservFileName=' + fileName
	})
	.then(response => {
		if (!response.ok) {
			throw new Error('Check network tab');
		}
		return response;
	})
	.then(response => response.text())
	.then(data => {
		document.body.innerHTML = data;
		// window.location.reload();
	})
	.catch(error => {
		console.error(error);
		parentElement.style.border = '2px solid red';
		const fileContent = document.getElementById("file-content-text");
		fileContent.innerHTML = "No file selected";
	});
});

const displayFile = ((clickedElement, parentElement) => {

	resetBorders();

	fetch('displayFile.py?webservFileName=' + encodeURIComponent(clickedElement.innerHTML), {
		method: 'GET'
	})
	.then(response => {
		if (!response.ok) {
			throw new Error('Check network tab');
		}
		return response;
	})
	.then(response => response.text())
	.then(data => {
		document.body.innerHTML = data;
	})
	.catch (error => {
		console.error(error);
		parentElement.style.border = '2px solid red';
		const fileContent = document.getElementById("file-content-text");
		fileContent.innerHTML = "No file selected";
	});
})


const uploadFile = ((event) => {
	event.preventDefault();
	
	const file = document.getElementById('file-content').files[0];

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

		try {
			const response = await fetch("goodTimes.py", {
				method: 'POST',
				body: formData
			});

			if (!response.ok) {
				throw new Error('Check network tab');
			}
			
			const textData = await response.text();

			document.body.innerHTML = textData;
			// window.location.reload();
		} catch (error) {
			console.error(error);
		}
	};

	reader.readAsArrayBuffer(file);
});
