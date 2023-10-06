const displayFile = ((clickedElement) => {
	console.log(clickedElement.innerHTML);

	fetch('/cgi-bin/displayFile.py', {
		method: 'POST',
		body: 'webservFileName=' + clickedElement.innerHTML
	})
	.then(response => response.text())
	.then(data => {
		document.body.innerHTML = data;
	})
	.catch (error => {
		console.error('Error:', error);
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
			const response = await fetch("/cgi-bin/goodTimes.py", {
				method: 'POST',
				body: formData
			});

			if (!response.ok) {
				throw new Error('Server response was not ok');
			}
			
			const textData = await response.text();

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

	fetch('/cgi-bin/goodTimes.py', {
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